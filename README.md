# Cannonball for x68

An unofficial X68000 port of [Cannonball](https://github.com/djyt/cannonball), the OutRun engine
by Chris White. X68000 port: **Mattsuan-Z@ColtMaz**.

**No ROMs are included.** As with Cannonball, you need your own OutRun (Revision B) arcade ROM files.
`CBX68 TEST` shows a display test that needs no ROM.

The program for the X68000 is on the Releases page. The text below is the README included with it.

## Building

- X68000: `M68K_XELF=<folder of the m68k-xelf toolchain> sh build_x68k.sh` makes `STAGE1.X`
  (rename it to `CBX68.X`). Needs m68k-xelf-gcc with the Human68k libraries.
- PC check (Linux, g++): `sh build_host.sh` builds `build/host/stage1` and runs the display test.
  With ROMs: `CBX68_ROMS=<rom folder>/ ./build/host/stage1 2000`.

## README (as included with the program)

```
Cannonball for x68  v0.1
========================

An unofficial port of Cannonball (the OutRun engine by Chris White) to the X68000.
X68000 port: Mattsuan-Z@ColtMaz
This is a non-commercial hobby project. It is not affiliated with, endorsed by or
connected to SEGA or the Cannonball team.
OutRun is a trademark of the SEGA Corporation.

NO ROMS ARE INCLUDED. As with Cannonball, you need your own OutRun (Revision B)
arcade ROM files.

Requirements
------------
- X68030 with a 68060 (tested on the XM6 emulator: 68060, clock x4, extended ROM on,
  "MPU speed-up" on, main memory 12MB, "VM sync to PC VSYNC" off)
- Human68k

How to use
----------
1. Put CBX68.X, TILEMAP.BIN and TILEPAT.BIN in one folder.
2. Make a folder ROMS in that folder and put the OutRun Revision B ROM files in it
   (the set Cannonball uses: epr-10380b.133, mpr-10371.9, opr-10185.11 ...).
   As in Cannonball, the files are found by their CRC32, so their names do not matter
   (8.3 names such as 10380B.133 are fine).
3. Start:  CBX68 0 30 manual both
   (a 5th argument gives another ROM folder, e.g.  CBX68 0 30 manual both A:\OUTRUN\)
If a file is missing or wrong, the program lists it (as Cannonball does) and stops.

Display test (no ROMs needed)
-----------------------------
  CBX68 TEST   (or TEST.BAT)
checks the display on your machine or emulator without any ROM file: a road that
bends and rises, two background layers scrolling at different speeds, shapes coming
up the road and text. The game does not run in this mode; all pictures are simple
shapes made by the program. Keys: 1-4 display, V 30/60 frames, ESC quit (the speed
figures are printed at the end, as in the game).

Keys
----
C credit / ENTER start / UP or Z accelerate / DOWN or X brake / LEFT RIGHT steer /
SPACE gear / 1-4 display / G roadside objects / H score display / R restart / ESC quit
(T: time stays at 99, A: drives by itself - check aids)

Not in v0.1: sound. Shadows are not shown. Where more than 32 sprites meet on one
line, parts of objects can be missing.

Credits
-------
X68000 port: Mattsuan-Z@ColtMaz
Cannonball (OutRun engine): Chris White and the Cannonball team
Video and sound hardware code in Cannonball: based on MAME (Aaron Giles and others)
The files in SOURCE/vendor come from Cannonball; some of them were changed for this port.
Their original notices are kept.

Licence
-------
Cannonball is copyright Chris White and the Cannonball team; this derivative is
released under the same licence (LICENSE.txt): not for sale or commercial use, and
the complete source is included (SOURCE).

----------------------------------------------------------------------------
Cannonball for x68 (日本語)

X68000向けの、Cannonball（Chris White氏によるOutRunエンジン）の非公式移植です。
X68000への移植: Mattsuan-Z@ColtMaz
非営利の個人制作で、SEGAおよびCannonballの開発者とは関係ありません。
OutRunはSEGAの商標です。

ROMは含まれていません。Cannonballと同じく、ご自身でお持ちのアーケード版OutRun
（Revision B）のROMファイルが必要です。

使い方
1. CBX68.X、TILEMAP.BIN、TILEPAT.BIN を同じフォルダーに置く
2. そのフォルダーに ROMS フォルダーを作り、ROMファイル（Cannonballと同じセット）を入れる
   （Cannonballと同じく中身のCRC32で探すので、ファイル名は自由です。例: 10380B.133）
3. CBX68 0 30 manual both で起動
   （5番目の引数でROMフォルダーを指定できます）
ファイルが無い・違う場合は、Cannonballと同様に一覧を表示して終了します。

表示テスト（ROM不要）
   CBX68 TEST （または TEST.BAT）
ROMなしで、お使いの機種・エミュレーターで表示が動くかを確認できます。曲がりと起伏の
ある道、速さの違う2枚の背景、道を近づいてくる図形、文字が表示されます。ゲームは動き
ません。絵はすべてプログラムが作った簡単な図形です。1〜4 表示切替、V 30/60、ESC 終了
（終了時に速さの数字を表示します）。

操作: C クレジット / Enter スタート / ↑かZ アクセル / ↓かX ブレーキ / ←→ ハンドル /
      Space ギア / 1〜4 表示 / G 景物 / H 点数表示 / R 再開始 / ESC 終了
v0.1では音は出ません。

クレジット: X68000への移植 Mattsuan-Z@ColtMaz ／ Cannonball Chris White氏とCannonballチーム ／
      Cannonballの表示・音源ハードウェア部分はMAME（Aaron Giles氏ほか）が元です。
      SOURCE/vendor はCannonballのファイルで、一部を移植のために変更しています（元の表記は残しています）。
許諾: Cannonballの許諾条件（LICENSE.txt）に従います。販売・商用利用は禁止、
      ソース一式（SOURCE）を同梱しています。
```
