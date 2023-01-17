from random import randint, seed
import sys
"""単一のseed値からテストケースを作成
- ファイルに書き出す場合
python3 gen.py <seed> ><output_file_path>
- そのままプログラムに渡す場合
python3 gen.py <seed> | python3 solve.py
"""
if len(sys.argv) == 2:
    try:
        seed(int(sys.argv[1]))
    except ValueError:
        print("[warning] seed needs to be integer.", file=sys.stderr)
print("50 2500")
for _ in range(50):
    print(" ".join(str(randint(1, 9)) for _ in range(50)))
