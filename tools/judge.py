import sys
from pathlib import Path

DR = [1, 0, -1, 0]
DC = [0, 1, 0, -1]
REMOVED = -float('inf')


class TestCase:
    N_FIXED = 50
    M_FIXED = 2500
    MIN_A = 1
    MAX_A = 9

    def __init__(self, path) -> None:
        with open(path) as f:
            input_ = f.readlines()
            self.N, self.M = map(int, input_[0].split())
            self.A = [[int(i) for i in line.split()]
                      for line in input_[1:]]

    def __str__(self) -> str:
        ret = ""
        ret += f"{self.N} {self.M}\n"
        for i in range(self.N):
            for j in range(self.N):
                ret += f"{self.A[i][j]}"
                ret += "\n" if j == self.N-1 else " "
        return ret


class Position:
    def __init__(self, r, c) -> None:
        self.r = r
        self.c = c

    def __eq__(self, other) -> bool:
        return self.r == other.r and self.c == other.c


class Output:
    r = []
    c = []
    isRm = []

    def __init__(self, path: Path, testcase) -> None:
        with open(path) as f:
            for i, line in enumerate(f):
                elems = line.split()
                isEmptyLine = not elems or (len(elems) == 1 and not elems[0])
                if isEmptyLine and len(self.r) == testcase.M:
                    continue
                if len(elems) != 3:
                    raise ValueError(f"line {i+1}: 不正な出力です {line}")
                if elems[0] == "1":
                    self.isRm.append(False)
                elif elems[0] == "2":
                    self.isRm.append(True)
                else:
                    raise ValueError(f"line {i+1}: 不正な出力です {line}")
                rv = int(elems[1])
                cv = int(elems[2])
                self.r.append(rv)
                self.c.append(cv)
                if len(self.r) > testcase.M:
                    raise ValueError(f"{testcase.M}回より多い操作を行おうとしました")
        self.a = [[] for _ in range(testcase.N)]
        for i in range(testcase.N):
            self.a[i] = testcase.A[i]

    def increment(self, testcase, cr: int, cc: int) -> None:
        self.a[cr][cc] += 1

    def remove(self, testcase, cr, cc):
        lis = []
        lis.append(Position(cr, cc))
        i = 0
        while i < len(lis):
            for j in range(4):
                nr = lis[i].r + DR[j]
                nc = lis[i].c + DC[j]
                np = Position(nr, nc)
                if 0 <= nr < testcase.N and 0 <= nc < testcase.N and self.a[nr][nc] == self.a[cr][cc] and not np in lis:
                    lis.append(np)
            i += 1
        if len(lis) >= self.a[cr][cc]:
            ret = len(lis) * self.a[cr][cc]
            for p in lis:
                self.a[p.r][p.c] = REMOVED
            return ret
        else:
            return 0


def calcScore(testcase, output) -> int:
    score = 0
    for i in range(len(output.r)):
        cr = output.r[i]
        cc = output.c[i]
        if cr < 0 or testcase.N <= cr or cc < 0 or testcase.N <= cc:
            raise ValueError(f"line {i+1}: 座標が範囲外です({cr}, {cc})")
        if output.a[cr][cc] == REMOVED:
            print(
                f"[warning] line {i+1}: すでに収穫済みの区画を操作しようとしました。無視します ({cr}, {cc})", file=sys.stderr)
        elif output.isRm[i]:
            scoreDiff = output.remove(testcase, cr, cc)
            if scoreDiff == 0:
                print(
                    f"[warning] line {i+1}: 収穫しようとした区画が条件を満たしていません。無視します ({cr}, {cc})")
            score += scoreDiff
        else:
            output.increment(testcase, cr, cc)
    return score


if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) < 2:
        exit("usage: py judge.py input_file_path output_file_path")
    inputFile = Path(args[0])
    outputFile = Path(args[1])
    testcase = TestCase(inputFile)
    output = Output(outputFile, testcase)
    score = calcScore(testcase, output)
    print(f"score: {score}")
