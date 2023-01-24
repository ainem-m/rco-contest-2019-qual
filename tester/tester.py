import subprocess
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor

# 設定

NUM_OF_TEAMS = 8
NUM_OF_CASES = 1000

max_process = 8
proc_list = []

CURRENT_DIR = Path("/Users/ainem/Desktop/rust/mintoku/rco-contest-2019-qual")
# 設定終わり

IN = CURRENT_DIR / "in"
OUT = CURRENT_DIR / "out"
ERR = CURRENT_DIR / "err"
IN.mkdir(exist_ok=True)
OUT.mkdir(exist_ok=True)
ERR.mkdir(exist_ok=True)
score_list = [[-1 for _ in range(NUM_OF_CASES)] for _ in range(NUM_OF_TEAMS)]


for team_num in range(NUM_OF_TEAMS):
    filename, lang, team = input().split()

    TEAM_OUT = OUT / team
    TEAM_OUT.mkdir(exist_ok=True)
    TEAM_ERR = ERR / team
    TEAM_ERR.mkdir(exist_ok=True)

    filename = Path(filename)
    command = ""
    if lang == "Python":
        command = f"python3 {filename}"
    elif lang == "Rust":
        subprocess.run(
            f"cargo build --bin {filename.stem} --release 2>/dev/null", shell=True, cwd=CURRENT_DIR)
        command = str(CURRENT_DIR.parents[0]) + \
            f"/target/release/{filename.stem}"
    elif lang == "C++":
        subprocess.run(
            f"g++ -std=gnu++17 -O2 -o ./{team}.out ./{filename} 2>/dev/null", shell=True, cwd=CURRENT_DIR
        )
        command = f"./{team}.out"
    elif lang == "Crystal":
        subprocess.run(
            f"crystal build {filename} --release 2>/dev/null", shell=True, cwd=CURRENT_DIR)
        command = "./" + {filename.stem}
    elif lang == "Java":
        subprocess.run(f"javac ./{filename}",
                       shell=True, cwd=CURRENT_DIR)
        command = f"java {filename}"

    # システムテスト実行
    for i in range(NUM_OF_CASES):
        proc = subprocess.Popen(
            f"{command} <{IN}/{i:04d}.txt >{TEAM_OUT}/{i:04d}.txt 2>{TEAM_ERR}/{i:04d}.txt",
            shell=True, cwd=CURRENT_DIR
        )
        proc_list.append(proc)
        print(f"\rrunning {i:04d}", end="")
        if (i + 1) % max_process == 0 or (i + 1) == NUM_OF_CASES:
            for subproc in proc_list:
                subproc.wait()
                # time.sleep(0.1)
        proc_list = []

    sum_score = 0

    # 出力からビジュアライザを用いて得点計算
    for i in range(NUM_OF_CASES):
        result = subprocess.run(
            f"java Judge {IN}/{i:04d}.txt {TEAM_OUT}/{i:04d}.txt",
            shell=True,
            capture_output=True,
            text=True,
        )
        score = int(result.stdout.split(":")[-1])
        sum_score += score
        score_list[team_num][i] = score
        print(f"\r{team} {i:04d} score: {score}", end="")
    score_list[team_num].append(sum_score)
    print(f"\r     {team} score: {sum_score}")

result_list = open("result.csv", "w")
print("team, ", end="", file=result_list)
for i in range(NUM_OF_CASES):
    print(f"{i:04d}", end=", ", file=result_list)
print("sum", file=result_list)

for team in range(NUM_OF_TEAMS):
    print(f"team{team+1}, ", end="", file=result_list)
    for case in range(NUM_OF_CASES + 1):
        print(score_list[team][case], ", ", end="", file=result_list)
    print("", file=result_list)

"""
    def run(i):
        with open(f"{IN}/{i:04d}.txt") as f:
            ret = subprocess.run(f"{command}", shell=True, stdin=f,
                                 cwd=CURRENT_DIR, text=True, capture_output=True)
        out = Path(f"{TEAM_OUT}/{i:04d}.txt")
        with open(out, mode="w") as f:
            f.write(ret.stdout)
        err = Path(f"{TEAM_ERR}/{i:04d}.txt")
        with open(err, mode="w") as f:
            f.write(ret.stderr)
    with ThreadPoolExecutor(max_workers=max_process) as executor:
        executor.map(run, range(NUM_OF_CASES))

    sum_score = 0

    def judge(i):
        ret = subprocess.run(
            f"java Judge {IN}/{i:04d}.txt {TEAM_OUT}/{i:04d}.txt", shell=True, text=True, capture_output=True)
        score = int(ret.stdout.strip().split(":")[1])
        print(i, ret.stdout.strip().split(":"))
        sum_score += score
        print(i, score, sum_score)
        score_list[team_num][i] = score
    with ThreadPoolExecutor(max_workers=max_process) as executor:
        executor.map(judge, range(NUM_OF_CASES))
    print(f"{team} score: {sum_score}")
"""
