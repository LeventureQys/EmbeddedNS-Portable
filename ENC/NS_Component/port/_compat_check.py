"""numpy 交叉验证 —— T1 兼容层自测第 4 项。

C 侧(arm_math_compat_test --emit-vectors)用种子 12345 的 xorshift64 生成 1000 组
伪随机输入, 写出:
    _compat_vectors.txt      输入向量(1000 x 256)
    _compat_vectors_c.txt    C 实现的正变换输出(1000 x 256, 压缩格式)
本脚本用 numpy.fft.rfft 对同一批输入做正向变换, 与 C 输出逐点比对。

对应关系(np.fft.rfft 返回 129 个复数):
    rfft[0].real  <-> pOut[0]
    rfft[128].real <-> pOut[1]
    rfft[k].real  <-> pOut[2k]      (k = 1..127)
    rfft[k].imag  <-> pOut[2k+1]    (k = 1..127)

判定: 最大相对误差 < 1e-5, 相对误差分母 max(|ref|, 1e-30)。

不安装任何 pip 包; numpy 由本地运行时提供。
"""

import os
import sys

import numpy as np

PORT_DIR = os.path.dirname(os.path.abspath(__file__))
TOL_REL = 1e-5
FLOOR = 1e-30


def load_matrix(path):
    with open(path, "r", encoding="ascii") as f:
        head = f.readline().split()
        ncases, nlen = int(head[0]), int(head[1])
        data = np.loadtxt(f, dtype=np.float64)
    data = np.asarray(data, dtype=np.float64)
    if data.shape != (ncases, nlen):
        raise SystemExit(f"向量文件 {path} 形状异常: {data.shape}, 期望 {(ncases, nlen)}")
    return ncases, nlen, data


def main():
    x_path = os.path.join(PORT_DIR, "_compat_vectors.txt")
    c_path = os.path.join(PORT_DIR, "_compat_vectors_c.txt")
    out_path = os.path.join(PORT_DIR, "arm_math_compat_numpy_check.txt")

    for p in (x_path, c_path):
        if not os.path.isfile(p):
            raise SystemExit(f"缺少向量文件: {p}\n请先运行 arm_math_compat_test --emit-vectors")

    ncases, nlen, x = load_matrix(x_path)
    _, _, c = load_matrix(c_path)

    lines = []
    lines.append("=== T1 numpy 交叉验证(C 实现 vs numpy.fft.rfft) ===")
    lines.append(f"numpy 版本: {np.__version__}")
    lines.append(f"样本组数: {ncases}, 每组长度: {nlen}")
    lines.append(f"判定阈值(最大相对误差): {TOL_REL:.0e}")
    lines.append("")

    # C 输出反解成 129 个复数
    nspec = nlen // 2 + 1  # 129
    c_spec = np.zeros((ncases, nspec), dtype=np.complex128)
    c_spec[:, 0] = c[:, 0]                     # DC
    c_spec[:, nspec - 1] = c[:, 1]             # Nyquist (置于 bin 128)
    k = np.arange(1, nspec - 1)
    c_spec[:, k] = c[:, 2 * k] + 1j * c[:, 2 * k + 1]

    ref = np.fft.rfft(x, axis=1)               # (ncases, 129) complex

    abs_err = np.abs(c_spec - ref)
    denom = np.maximum(np.abs(ref), FLOOR)
    rel_err = abs_err / denom

    idx = np.unravel_index(int(np.argmax(rel_err)), rel_err.shape)
    max_rel = float(rel_err[idx])
    max_abs = float(np.max(abs_err))
    # 仅在参考幅度显著时统计, 避免接近零的分量把相对误差放大
    sig = np.abs(ref) > 1e-6
    max_rel_sig = float(np.max(rel_err[sig])) if np.any(sig) else 0.0

    lines.append(f"最大相对误差(全部 129 x {ncases} 点) = {max_rel:.6e}  "
                 f"@ 组 {idx[0]}, bin {idx[1]}")
    lines.append(f"最大相对误差(|ref| > 1e-6 的分量)   = {max_rel_sig:.6e}")
    lines.append(f"最大绝对误差                        = {max_abs:.6e}")
    lines.append("")

    # 抽样打印前 3 组的前 4 个 bin, 便于人工核对
    lines.append("抽样(组 0, bin 0/1/2/128):")
    for b in (0, 1, 2, nspec - 1):
        lines.append(f"  bin {b:3d}: C = {c_spec[0, b].real:+.9e} "
                     f"{c_spec[0, b].imag:+.9e}j   "
                     f"numpy = {ref[0, b].real:+.9e} {ref[0, b].imag:+.9e}j")
    lines.append("")

    # 边界确证: 与 C 侧边界测试同源的解析对照(恒 1 与 delta 的 rfft)
    one = np.ones(nlen, dtype=np.float64)
    dlt = np.zeros(nlen, dtype=np.float64)
    dlt[0] = 1.0
    r_one = np.fft.rfft(one)
    r_dlt = np.fft.rfft(dlt)
    lines.append("解析对照:")
    lines.append(f"  x[n]==1      -> rfft[0]={r_one[0].real:.9g} (期望 256), "
                 f"rfft[128]={r_one[128].real:.9g} (期望 0), "
                 f"其余最大幅度={np.max(np.abs(r_one[1:128])):.3e}")
    lines.append(f"  x[n]==delta  -> rfft[0]={r_dlt[0].real:.9g} (期望 1), "
                 f"rfft[128]={r_dlt[128].real:.9g} (期望 1), "
                 f"rfft[k] 全部 = 1+0j: "
                 f"{bool(np.allclose(r_dlt[1:128], 1.0 + 0.0j, atol=1e-12))}")
    lines.append("")

    ok = max_rel < TOL_REL
    lines.append(f"判定: {'PASS' if ok else 'FAIL'}")
    text = "\n".join(lines) + "\n"

    with open(out_path, "w", encoding="utf-8") as f:
        f.write(text)

    # 追加到 T1 自测留档
    log_path = os.path.join(PORT_DIR, "arm_math_compat_test_output.txt")
    if os.path.isfile(log_path):
        with open(log_path, "a", encoding="utf-8") as f:
            f.write("\n=== numpy 交叉验证(由 _compat_check.py 追加) ===\n")
            f.write(f"numpy {np.__version__}, {ncases} 组 x {nspec} 个复数值\n")
            f.write(f"最大相对误差 = {max_rel:.6e} (阈值 {TOL_REL:.0e})\n")
            f.write(f"最大绝对误差 = {max_abs:.6e}\n")
            f.write(f"判定: {'PASS' if ok else 'FAIL'}\n")

    print(text, end="")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
