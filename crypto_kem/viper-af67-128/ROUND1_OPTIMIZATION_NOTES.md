# Viper-128 第一轮 Cortex-M4 流水线优化记录

日期：2026-06-22

范围：只修改 `viper-af67-128/m4fspeed` 与 `viper-af67-128/m4fstack`，未修改 192/256 参数集，也未重写 NTT 汇编。

## 本轮完成内容

1. Decaps 点积路由到 `dot_m4ntt`（前序 P0 修复）。
2. NTT 点积的第一项乘积直接写入 accumulator，删除 `prod_ntt -> memcpy(acc)`；后续项复用 `a_ntt` 作为乘积 scratch。
3. A 多项式的 12-bit parse 与模 q 中心化合并为一次扫描，stream NTT 不再保留 `a_poly` 后再调用 `center_poly`。
4. 公钥多项式改成逐个处理：按索引生成 packed dpk，直接完成 9-bit 公钥解包、dpk 重构、中心化、NTT 和点积累加，不再分配完整 `bhat[K][N]`。
5. stream NTT 每完成一个输出就立即 inverse NTT 并调用 emit 回调：
   - KeyGen 立即量化并写入公钥；不再保存完整 `b[K][N]`。
   - Encaps 立即量化、dither、pack 到密文；不再保存完整 `u[K][N]`。
   - 重加密立即量化、pack-and-compare；不再保存完整 `u[K][N]`。
6. Decaps 在 `crypto_kem_dec` 中只生成一次 dither，同一份数据传给 PKE 解密和重加密校验，删除第二次 `ViperDither` SHAKE。
7. 保留两套实现的策略差异：
   - `m4fspeed` 仍一次生成并复用秘密向量的 NTT 表示。
   - `m4fstack` 仍按乘积重新做秘密 NTT，避免常驻 `s_ntt[K][N]`。

## 主要被删除的热路径缓冲/遍历

- 首项乘积的 `prod_ntt[256]` 和 1 KB `memcpy`。
- stream A 路径的 `a_poly[256] -> center_poly[256]` 二次写读。
- Encaps/重加密的完整 `bhat[K][N]`。
- KeyGen 的完整 `b[K][N]`。
- Encaps/重加密的完整 `u[K][N]`。
- Decaps 重加密阶段的第二次 dither SHAKE。

## 已完成验证

- `m4fspeed`、`m4fstack` 的 `test`、`speed`、`stack`、`hashing`、`testvectors` ELF 均编译成功。
- 10,000 组主机随机等价性测试通过：
  - 新旧 A parse/center 逐系数一致；
  - 新旧 9-bit 公钥量化打包逐字节一致；
  - 新旧公钥+dpk 重构中心化逐系数一致。
- 用 schoolbook oracle 替代 ARM NTT 的主机 PKE 流水线测试通过：KeyGen、Encaps、Decaps、重加密成功校验、篡改密文拒绝。
- ARM ELF 反汇编确认热路径调用 `matvec_stream_m4ntt`、`matTvec_dot_stream_m4ntt` 和 `dot_m4ntt`；Decaps 只保留一次 `viper_gen_dither_bytes` 调用。
- 方案静态库 `.text`：
  - `m4fspeed`：27,656 -> 28,060 bytes（+404 bytes）。
  - `m4fstack`：26,720 -> 27,324 bytes（+604 bytes）。
  - 增量来自 128 专用的 packed dpk/9-bit 公钥融合路径与 emit 回调；是否保留这项体积换流水线的取舍，以板上 cycle/stack 结果决定。

## 尚待上板验证

当前执行环境缺少 `/dev/ttyUSB0`，因此没有声称板上 KEM 或 cycle/stack benchmark 已通过。连接开发板后应执行：

```sh
venv/bin/python test.py --platform stm32f4discovery viper-af67-128
venv/bin/python benchmarks.py --platform stm32f4discovery viper-af67-128
```

本轮不填写预期 cycle 数字；以重新采集的板上结果为准。

## 下一轮候选

- 为 12-bit A -> centered int16 增加专用汇编解包。
- 为 9-bit ciphertext + dither -> centered int16 增加专用汇编解包。
- 将 inverse NTT 后的 mask、dither、量化、pack/compare 进一步下沉成专用汇编接口。
- 在规范允许且测试向量保持不变的前提下，评估 A/dpk 的增量 SHAKE 流。
