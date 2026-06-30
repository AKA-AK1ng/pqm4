# Viper-E8-128 Cortex-M4 优化记录

日期：2026-06-24

范围：`crypto_kem/viper_e8-128/{m4fspeed,m4fstack}`。本文按实际实施顺序记录问题、修改思路、验证状态和后续实验。除非明确写出板上 cycles，否则所有性能收益都只是依据热路径结构作出的预期，不能替代 STM32 实测。

本轮约定的正确性门槛是显式执行
`make PLATFORM=nucleo-l4r5zi ..._test.elf` 并构建成功；不要求上板运行或 QEMU 模拟。板上只用于后续获取真实性能数据。

## 0. 基线与问题定位

基线来自 `FVE8_newViperParams.md`：

| 实现 | KeyGen | Encaps | Decaps | KeyGen stack | Encaps stack | Decaps stack |
|---|---:|---:|---:|---:|---:|---:|
| m4fspeed | 407365 | 688841 | 787677 | 12096 B | 15360 B | 15728 B |
| m4fstack | 433804 | 715822 | 814721 | 10528 B | 13792 B | 14160 B |

与旧 `viper-af67-128` 的 `m4fspeed` 相比，新参数分别慢 9.1%、27.7%、29.3%。但 ref 只慢 1.2%、5.5%、5.7%，说明主要差距来自 M4 优化路径退化，而不是 E8 算法本身。

发现的问题：

1. E8 encode 对一个 block 的同一 label 调用 8 次 `e8_unrank()`，每个坐标重复一次完整 unrank。
2. KeyGen/Encaps/reencrypt 重新保存完整 `A[K][K]`，没有采用旧版的流式 A 展开。
3. Encaps/reencrypt 分别执行 `A^T*r` 和 `bhat^T*r`，导致 `r` 被重复中心化和 forward NTT。
4. reencrypt 先生成临时 `packed_u`/`packed_v`，随后再次遍历执行比较。
5. `m4fspeed` 的 NTT 累加引入额外 `prod_ntt[256]` 和 memcpy；inverse NTT 后也经过不必要的临时数组。
6. E8 目录保留了旧汇编文件和开关，但主路径没有调用；其中 eta1 汇编不适用于新参数的 eta2，其余 t9 路径也使用旧版 packed-dither 接口，不能直接无条件启用。

## 1. 合并 E8 encode 的重复 unrank

### 修改思路

每个 block 只调用一次 `e8_unrank(d, label)`，然后直接把 `d[0..7]` 映射成八个系数。公开的 `viper_e8_label_to_coeff()` 保留，用于 API 和测试，但热路径不再逐坐标调用它。

### 预期收益

每个 active block 的 unrank 次数从 8 降到 1，即 encode/unrank 主体工作减少 87.5%。该修改不改变 label 排序、码字或 wire format。

### 验证

- `m4fspeed` test/testvectors ELF：交叉编译通过。
- `m4fstack` test/testvectors ELF：交叉编译通过。
- 板上 cycles：待运行。

## 2. 恢复流式 A 展开并复用 secret NTT

### 修改思路

复用现有 `matvec_stream_m4ntt()` 和 `matTvec_dot_stream_m4ntt()`：

- KeyGen：逐个 SHAKE 展开 A 多项式；secret vector 只做一次 forward NTT；不保存完整 A。
- Encaps/reencrypt：融合 `A^T*r` 与 `bhat^T*r`，共享同一份 `r_NTT`。
- dpk 仍按原 domain separation 逐多项式生成，保持输出兼容。

对于 E8-128，NTT 累加安全界限为：

`K * N * (q/2) * eta = 2 * 256 * 2048 * 2 = 2097152 < Q1Q2/2 = 12785024`。

因此当前流式函数的动态 bound 检查在合法参数和合法 secret sampler 输出下不会进入失败分支。

### 预期收益

- 去掉完整 A 的存储和 parse 后的中间搬运。
- Encaps/reencrypt 少一次 secret vector 的中心化及 K 次 forward NTT。
- 后续可在 inverse NTT 的 emit 点直接融合 PK/U pack。

### 验证

- 两种实现的 test/testvectors ELF 均交叉编译通过。
- 板上 cycles：待运行。

## 3. 融合 reencrypt 的 quantize/pack/compare

### 修改思路

为 E8-128 实际使用的 `T_U=9`、`T_V=4` 编写专用 compare：量化后直接组成目标字节，与 ciphertext XOR/OR 累计差异，不写 `packed_u` 或 `packed_v`，也不再调用第二遍 `diff_bytes()`。

比较保持固定循环次数和无 secret-dependent early return。

### 预期收益

- 删除 reencrypt 中 416 B 临时 packed buffer。
- U/V 密文各减少一次完整内存写入和再次读取。
- 主要改善 Decaps，因为 FO check 每次都执行完整 reencrypt。

### 验证

- 两种实现的 test/testvectors ELF 均交叉编译通过。
- 专用 pack 与 fused compare 差分测试：待补充。
- 板上 cycles：待运行。

## 4. 去除 m4fspeed NTT 累加临时数组

### 修改思路

恢复旧版安全的 destructive scratch 使用：

- 第一个乘积直接写入 accumulator；
- 后续乘积复用当前局部 `a_ntt` 作为乘法输出，再累加；
- inverse NTT 直接写目标 `vpoly`。

`a_ntt` 在每次循环后不再使用，因此该复用不改变运算结果。`m4fstack` 原本已经采用这种形式，无需修改。

### 预期收益

减少 1 KiB 临时栈、一次首乘积 memcpy，以及 inverse NTT 后的 512 B 临时数组和复制。

### 验证

- m4fspeed test/testvectors ELF：交叉编译通过。
- 板上 cycles/stack：待运行。

## 5. Decaps 从 packed 数据流式生成 NTT 输入

### 修改思路

恢复旧实现的 `dot_dec_stream_m4ntt()` 调度方式：

- secret callback 从 secret key 解包单个多项式并原地中心化；
- U callback 对单个 t9 多项式执行 unpack + reconstruct 并原地中心化；
- 每个多项式立即 forward NTT 和乘加，同一个 512 B centered scratch 在 secret/U 之间复用。

原路径同时保存 `s[K][N]` 和 `u[K][N]`，随后 `viper_dot()` 再次逐向量中心化。新路径不保留这两个向量，也不执行二次中心化。

### 预期收益

- PKE decrypt 栈减少约 `2*K*N*sizeof(uint16_t) = 2048 B`，实际 KEM stack 仍需上板测量。
- 减少四次 256-coefficient 中间数组写入/读取。
- 为下一步把 t9 unpack/reconstruct/center 合为汇编或紧凑 C 内核提供单一入口。

### 验证

- 两种实现的 test/testvectors ELF 均交叉编译通过。
- 板上 stack 和 cycles：待运行。

## 6. inverse NTT 后立即 emit U

### 修改思路

在流式 `A^T*r` 调度中加入 emit callback。每个 U 多项式完成 inverse NTT 后立即量化并写入 ciphertext，或在 FO 重加密中立即与 ciphertext 比较。同时，`bhat` 也改为按需从 PK 展开 centered 多项式，不再保存整个向量。

新 Encaps/reencrypt 热路径只保留 `r`、一个 NTT accumulator 和一个 centered scratch，不再保留 `u[K][N]`、`bhat[K][N]`。

### 预期收益

- 再删除两个 K-polynomial 中间向量，约 2048 B 调用方栈。
- inverse NTT 输出只扫描一次，直接进入 pack/compare。
- PK reconstruct 与 dot-product forward NTT 紧邻，改善局部性。

### 验证

- `PLATFORM=nucleo-l4r5zi` 的 m4fspeed/m4fstack test ELF 构建通过。
- speed/stack ELF 构建通过。

## 7. packed dither 与兼容汇编恢复

### 参数兼容性判断

不能直接复用的旧汇编：

- eta1 decode：新 Viper-E8-128 使用 eta2，禁止调用。

可以复用的旧汇编：

- q=4096 的 12-bit secret pack/unpack；
- `T_PK=9`、2-bit dpk 的 PK pack/reconstruct；
- `T_U=9`、3-bit dither 的 U pack/reconstruct/compare。

新参数的 `T_V=4`、8-bit dither 在旧版本不存在，因此新增专用 C 内核，不能误用旧 t3+d9 汇编。

### 修改思路

- SHAKE 直接生成 448 B packed dither，不再扩展为 1536 B `uint16_t du/dv`。
- U 的 inverse-NTT emit 直接调用 t9+d3 汇编。
- Decaps 的 U callback 直接调用融合 unpack/reconstruct/center 汇编。
- PK 使用 packed 64 B dpk，并调用 t9+d2 汇编。
- secret key 使用通用 12-bit（与 eta 无关）汇编 pack/unpack。
- 为 V 新增 t4+d8 pack、reconstruct、constant-time compare。

packed dither 与旧 expanded dither 使用完全相同的 SHAKE 输入、输出长度和 bit 顺序；这里只是取消展开步骤，不修改密文格式。

### 预期收益

- 每次 Encaps/Decaps/reencrypt 删除 768 次 dither coefficient 展开写入。
- dither 调用方栈从 1536 B 降至 448 B。
- 恢复 U、PK、secret 热路径的 Cortex-M4 专用汇编。

### 验证

- `make PLATFORM=nucleo-l4r5zi ..._m4fspeed_test.elf`：通过。
- `make PLATFORM=nucleo-l4r5zi ..._m4fstack_test.elf`：通过。
- 两种实现的 speed/stack ELF：构建通过。
- 构建无新增 compiler warning。

## 下一步

1. 增加 t9+d3 汇编与 packed-dither reference 的差分固件；按项目要求只需确认 NUCLEO test ELF 可构建，不要求 QEMU。
2. 为 eta2 写专用 sampler + prepared NTT 路径；不能误用旧 eta1 汇编。
3. profile E8 decode；若占比明显，再把 alpha=2048 的通用除法替换为经过差分验证的定点 shift/rounding，并评估专用 E8 nearest-point 内核。
4. 实际测速后将每一步的 A/B cycles 回填本文。

## 8. 第一轮实测结果与 LightSaber 差距复核

`FVE8_newViperParams.md` 更新后的 128 cycles：

| 实现 | KeyGen | Encaps | Decaps | 相对本文件初始基线 |
|---|---:|---:|---:|---:|
| m4fspeed | 384901 | 539853 | 587879 | -5.5% / -21.6% / -25.4% |
| m4fstack | 411299 | 541995 | 590521 | -5.2% / -24.3% / -27.5% |

与旧 `viper-af67-128` benchmark 比较，m4fspeed Encaps 已基本持平（539853 对 539605），Decaps 反而快约 3.5%。剩余差距主要集中在 KeyGen 和 E8 codec，而不再是矩阵乘调度。

### 已与 LightSaber 同步的思路

- secret vector 的 NTT 复用；
- 流式 A 展开，不保存完整矩阵；
- NTT-domain 乘积累加；
- inverse NTT 后立即 pack/compare；
- Encaps 与 FO reencrypt 共用同一调度；
- Decaps 从 packed SK/CT 直接生成 NTT 输入；
- destructive NTT scratch，避免额外乘积数组和 memcpy。

### 没有照搬的 LightSaber 路径

1. LightSaber m4fstack 的 16-bit/32x16 hybrid CRT 调度以增加重复变换换取更低栈。Viper 目前的 m4fstack 已采用流式 scratch，但尚未照搬 hybrid。特别是 q=8192 的 384/512 参数必须重新证明每个单模数分支的范围，不能直接复制。
2. LightSaber 使用一个 incremental SHAKE context 顺序生成矩阵。Viper 对每个矩阵元素使用带 domain/i/j 的独立输入；直接改成连续 squeeze 会改变 A 和测试向量，因此不属于无损优化。
3. CBD 与 NTT 可以进一步融合，但当前采样之后只有一次中心化扫描，预期收益小于 E8 codec 和 KeyGen bound 扫描，暂不优先写新汇编。

结论：m4fspeed 的高层矩阵乘思路已经与 LightSaber 对齐；后续不应盲目移植更多 Saber 汇编，而应优先优化 E8 codec、静态界限和各参数 packing。

## 9. rate=1 E8 rank/unrank 直接映射

### 问题发现

128/192/256 的 `VIPER_E8_RATE=1`，每个 block 只有 8 payload bits，residue modulus 为 4。原通用实现仍维护 288 B 动态 count cache，并在 rank/unrank 中执行嵌套枚举和反复 cache 查询。

### 等价关系

对 modulus 4 的 lexicographic E8 residues：

- label 高 2 bits 直接选择 `d[0]`；
- 低 6 bits 直接选择 `d[1..6]` 的 high residue bit；
- `d[7]` 是使八个 high bits 总奇偶为偶数的唯一值。

对全部 256 labels 穷举比较，直接映射与原通用 unrank 完全一致。

### 修改

- rate=1 编译为直接 bit mapping rank/unrank；
- rate=2 的 384/512 保留原通用算法；
- rate=1 不再编译动态 count cache，因此同时删除 288 B `.bss`。

### 验证

- 128/192/256 全部 codec 回环测试通过；
- 128 的全部 256 labels 与逐坐标 reference 完全一致；
- m4fspeed/m4fstack NUCLEO test ELF 构建通过。

## 10. alpha=2048 的 E8 舍入专用化

五组参数的 `VIPER_E8_ALPHA` 都等于 2048。原 decoder 将其作为通用除数传入 floor-division，再计算 quotient/remainder 和两个候选距离。

保持“恰好 half 时选择较小整数”的原规则，可化为：

- `a >= 0`：`(a + 1023) / 2048`；
- `a < 0`：`(a - 1024) / 2048`，利用 C 的向零截断。

已对 `[-8192,8191]` 全范围穷举验证与旧函数一致。修改后编译器可直接针对常数 2048 生成 shift/add 序列，不再保留通用 `%` 路径。

## 11. 同步到 m4fstack 与全部安全参数

同步范围：128、192、256、384、512 的 m4fspeed 和 m4fstack。

所有参数现在共享：

- 流式 A 展开；
- Encaps/reencrypt 的 NTT 复用与 inverse-NTT emit；
- Decaps packed-input 调度；
- packed dither；
- constant-time streaming pack/compare；
- E8 codec 优化；
- destructive NTT scratch。

### 参数专用后端选择

| 参数 | legacy secret12 asm | PK t9+d2 asm | U t9+d3 asm | 其他 pack |
|---|---:|---:|---:|---|
| 128 | 是 | 是 | 是 | V=t4+d8 专用 C |
| 192 | 是 | 否（PK=t10） | 是 | PK/V packed C |
| 256 | 是 | 否（PK=t10） | 否（U=t10） | PK/U/V packed C |
| 384 | 否（qlog=13） | 否 | 否 | 全部 packed C |
| 512 | 否（qlog=13） | 否 | 否 | 全部 packed C |

eta1 decode 汇编在所有新参数的生产 sampler 中保持禁用。192/256 新增汇编源只承载与参数兼容的 qlog12/t9 函数；384/512 不链接 legacy Viper packing 汇编。

### 静态 NTT 界限

KeyGen 和 Encaps 的 hot stream 只乘“dense polynomial × bounded secret”。对五组参数分别有：

`K*N*eta*(q/2) < Q1Q2/2`。

最紧的是 512：`9*256*1*4096 = 9437184 < 12785024`。因此生产流式路径删除运行时 `max_abs` 扫描，改由 `_Static_assert` 防止未来参数越界。通用 dense×dense API 仍保留动态检查和 fallback。

### 最终构建验证

以下 10 个目标均以 `PLATFORM=nucleo-l4r5zi` 构建通过且无新增编译警告：

- 五组参数 × `m4fspeed_test.elf`；
- 五组参数 × `m4fstack_test.elf`。

五组参数的 codec 测试也全部通过；rate=2 使用分层 label 采样覆盖所有 block。

## 后续测量重点

1. 重新生成五组 speed/stack/hashing 数据，尤其确认 384/512 从完整 A 矩阵切换为流式后的收益。
2. 确认最新 stack 表；当前 `FVE8_newViperParams.md` 的 stack/hash 百分比仍像是第一轮前数据。
3. 若 m4fstack 的栈仍需继续压缩，再单独研究 LightSaber 的 hybrid CRT 路径，并为 q=4096/q=8192 分别做范围证明。
4. 若 rate=2 codec 占比明显，再推导 modulus 8 的直接 rank/unrank；不要使用 65536-entry lookup table 直接换速度。
