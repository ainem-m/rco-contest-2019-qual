from random import randint, seed
import sys

"""使い方
1)設定
SEEDを好きな値に変更してください。(SEEDが同じであれば、毎回同じ乱数が生成されます。)
MAXは通常変更の必要はないはずです。
2)実行
python3 make_seeds.py <n> >seeds.txt
でn行のseed値が記されたseeds.txtが作成されます。
"""

# ここを変更する
SEED = 202103125
# 好みに応じて変更
MAX = 10**9

seed(SEED)
n = int(sys.argv[1])

for _ in range(n):
    print(randint(0, MAX))
