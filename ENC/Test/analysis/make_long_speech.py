# -*- coding: utf-8 -*-
"""合成 >=10 分钟的连续测试语音。

数据源：Test_Audio/AudioSample-48000hz/*.wav (16bit / 2ch / 48kHz)
        Test_Audio/voice/{2,3,m-k}.wav    (48kHz)
做法：三轮拼接，每轮内顺序轮转；段间 10 ms 交叉淡化抑制拼接咔哒。
输出：ENC/Test/data/input_long/merged_speech_10min.wav
"""
from __future__ import annotations

import os
import sys
import wave

import numpy as np

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
SRC_A = os.path.join(ROOT, "Test_Audio", "AudioSample-48000hz")
SRC_V = os.path.join(ROOT, "Test_Audio", "voice")
OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "data", "input_long")
OUT_WAV = os.path.join(OUT_DIR, "merged_speech_10min.wav")

SR = 48000
XFADE = int(0.010 * SR)          # 10 ms
TARGET_S = 600.0                 # 至少 10 分钟


def read_wav(path: str) -> tuple[np.ndarray, int, int]:
    """返回 (float32[frames, ch], rate, sampwidth)。"""
    with wave.open(path, "rb") as w:
        n = w.getnframes()
        ch = w.getnchannels()
        sw = w.getsampwidth()
        rate = w.getframerate()
        raw = w.readframes(n)
    if sw != 2:
        raise SystemExit(f"unsupported sample width {sw*8}bit: {path}")
    a = np.frombuffer(raw, dtype="<i2").astype(np.float32)
    a = a.reshape(-1, ch)
    return a, rate, sw


def to_stereo(a: np.ndarray) -> np.ndarray:
    if a.shape[1] == 1:
        return np.repeat(a, 2, axis=1)
    if a.shape[1] >= 2:
        return a[:, :2]
    raise SystemExit("bad channel count")


def xfade_concat(chunks: list[np.ndarray]) -> np.ndarray:
    """顺序拼接，相邻段之间做交叉淡化（各取 XFADE 样本）。"""
    out = chunks[0]
    for nxt in chunks[1:]:
        n = min(XFADE, out.shape[0], nxt.shape[0])
        if n <= 1:
            out = np.concatenate([out, nxt], axis=0)
            continue
        ramp = np.linspace(0.0, 1.0, n, dtype=np.float32)[:, None]
        tail = out[-n:] * (1.0 - ramp)
        head = nxt[:n] * ramp
        out = np.concatenate([out[:-n], tail + head, nxt[n:]], axis=0)
    return out


def main() -> int:
    names = [f"{i}.wav" for i in range(1, 10)]
    pool: dict[str, np.ndarray] = {}
    for nm in names:
        p = os.path.join(SRC_A, nm)
        a, rate, _ = read_wav(p)
        if rate != SR:
            raise SystemExit(f"rate mismatch {rate} != {SR}: {p}")
        pool[nm] = to_stereo(a)

    voice: dict[str, np.ndarray] = {}
    for nm in ("2.wav", "3.wav", "m-k.wav"):
        p = os.path.join(SRC_V, nm)
        a, rate, _ = read_wav(p)
        if rate != SR:
            print(f"[skip] {nm}: {rate} Hz != {SR} Hz")
            continue
        voice[nm] = to_stereo(a)

    base_total = sum(pool[n].shape[0] for n in names) / SR
    voice_total = sum(v.shape[0] for v in voice.values()) / SR
    per_round = base_total + voice_total
    rounds = max(1, int(np.ceil(TARGET_S / per_round)))
    print(f"[info] 单轮时长 {per_round:.2f}s -> 需要 {rounds} 轮")

    rot = [0, 4, 8]
    timeline: list[np.ndarray] = []
    for r in range(rounds):
        order = names[rot[r % len(rot)]:] + names[:rot[r % len(rot)]]
        for nm in order:
            timeline.append(pool[nm])
        for nm in voice:
            timeline.append(voice[nm])

    merged = xfade_concat(timeline)
    dur = merged.shape[0] / SR
    print(f"[info] 合成时长 {dur:.2f}s = {dur/60:.2f} min, {merged.shape[0]} 帧, {merged.shape[1]} 声道")

    peak = float(np.max(np.abs(merged)))
    if peak > 32767.0:
        merged *= 32767.0 / peak
        print(f"[warn] 峰值 {peak:.1f} 超界, 已整体缩放到满量程")

    os.makedirs(OUT_DIR, exist_ok=True)
    pcm = np.clip(np.rint(merged), -32768, 32767).astype("<i2")
    with wave.open(OUT_WAV, "wb") as w:
        w.setnchannels(2)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(pcm.tobytes())

    print(f"[ok] 写出 {os.path.relpath(OUT_WAV, ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
