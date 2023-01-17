from pathlib import Path
import sys
import subprocess

"""
seeds.txtに書かれたseed値の数だけテストケースを作成するプログラムです。
使い方
1)設定
OUTPUT_DIRにテストケースの書き出し先を設定します。
2)実行
python3 gen_from_seeds.py seeds.txt
"""
OUTPUT_DIR = "in"


output_dir = Path(OUTPUT_DIR)
if not output_dir.exists():
    output_dir.mkdir()

path = Path(sys.argv[1])
assert path.exists()

with open(path) as f:
    for i, line in enumerate(f.readlines()):
        try:
            line = int(line)
        except ValueError:
            exit("seeds must be integer")
        subprocess.run(
            f"python3 gen.py {line} 1>{OUTPUT_DIR}/{i:04d}.txt", shell=True)
