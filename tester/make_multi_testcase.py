import subprocess
from concurrent.futures import ThreadPoolExecutor


def run(i: int):
    subprocess.run(
        f"java Generator -{seeds[i]}  > ../in/{i:04d}.txt", shell=True, stdout=subprocess.PIPE)
    # stdout=の指定がないとターミナルにリダイレクトとは別に結果が出力される（なぜ？）


seeds = []
with open("../seeds.txt") as f:
    seeds = list(f.readlines())

with ThreadPoolExecutor() as executor:

    _ = executor.map(run, range(1000))


print("fin")
