# Viper-128 第二轮 C 层结构优化记录

日期：2026-06-22

范围：只修改 `viper-af67-128/m4fspeed` 与 `viper-af67-128/m4fstack`。本轮没有新增或重写汇编，也没有修改 192/256 参数集。

## 测量基线说明

`VOPT.md` 中保存的 359,414 / 523,487 / 557,417 cycles 仅作为进入第二轮前的历史参考。当前环境没有 `/dev/ttyUSB0`，因此本轮没有用旧数字冒充当前源码的板上结果。

## 本轮完成内容

1. 新增 `dot_dec_stream_m4ntt` 专用解密点积：
   - 12-bit secret key 直接解包成 centered int16；
   - 9-bit U + 3-bit dither 直接重构成 centered int16；
   - 每个向量分量立即 forward NTT、base multiplication 和 accumulator 累加；
   - 只做一次 inverse NTT；
   - 删除解密热路径中的完整 `s[K][N]`、`u[K][N]`、`center_poly` 和 `max_abs_poly`/fallback 扫描。
2. eta=1 采样专门化为固定 LUT `{0, 1, -1, 0}`，继续读取原 512-byte SHAKE 输出的每字节低 2 bit，保持 seed -> secret 映射不变。
3. `m4fspeed` 的 eta=1 采样直接生成 centered polynomial 并 forward NTT：
   - KeyGen 在 NTT 前直接写 12-bit secret key；
   - Encaps/重加密只保留 `s_ntt/r_ntt`，不再先保存 `vpolyvec` 后重新中心化和 NTT；
   - `m4fstack` 保留按乘积重新 NTT 的低常驻内存策略。
4. 固定 2-bit DPK 改为每 8 个系数直接加载两个字节并移位提取，删除热路径中的 8 次 `bitreader_read(..., 2)`。
5. 增加固定 12-bit secret pack，以及 12-bit -> centered int16 unpack；每两个系数直接处理 3 字节。
6. 合并最终系数遍历：
   - Encaps/重加密使用 `encode + add` 单次遍历；
   - Decaps 使用 `subtract + decode` 单次遍历，并按 8 个系数直接生成消息字节。
7. `finish_acc_ntt()` 直接 inverse NTT 到目标数组，再原地 mask；删除局部 512-byte 输出和复制。
8. KEM 失败密钥选择改为 `memcpy(kr)` 后调用常数时间 `cmov`，删除 `ok ? kr : z` 条件选择。

## 栈帧变化（ARM ELF 反汇编）

- `viper_pke_dec`：约 3,152 B -> 1,096 B（含保存寄存器）。
- 解密点积：约 4,152 B -> 3,624 B（含保存寄存器）。
- 两层嵌套合计：约 7,304 B -> 4,720 B，减少约 2,584 B。

这里是编译后栈帧静态观察，不替代 pqm4 的板上 stack benchmark。

## 代码体积

- 当前 `m4fspeed` 静态库 `.text`：27,724 B。
  - 相对第一轮记录的 28,060 B：-336 B。
  - 相对最初的 27,656 B：+68 B。
- 当前 `m4fstack` 静态库 `.text`：27,376 B。
  - 相对第一轮记录的 27,324 B：+52 B。
  - 相对最初的 26,720 B：+656 B。

## 已完成验证

- `m4fspeed` 与 `m4fstack` 均通过主机 schoolbook-oracle PKE 流水线测试：KeyGen、Encaps、Decaps、重加密成功校验和篡改密文拒绝。
- 固定 12-bit pack/unpack 与原通用路径逐字节/逐系数一致。
- 9-bit U + 3-bit dither centered 重构与原重构后中心化逐系数一致。
- 两套实现的 eta=1 输出均与原逐 bit 公式逐系数一致；同时检查了 `m4fspeed` 写出的 packed secret key。
- ARM ELF 确认 `crypto_kem_dec` 调用常数时间 `cmov`，PKE 解密调用 `dot_dec_stream_m4ntt`。

## 尚待开发板验证

连接开发板后重新采集，不覆盖历史基线：

```sh
venv/bin/python test.py --platform stm32f4discovery viper-af67-128
venv/bin/python benchmarks.py --platform stm32f4discovery viper-af67-128
```

板上结果确认后，再决定进入哪一条专用汇编路径。
