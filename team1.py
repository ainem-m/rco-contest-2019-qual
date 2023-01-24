import copy
from collections import defaultdict
import heapq
import numpy as np
import time
start = time.time()


class UnionFind():
    def __init__(self, n):
        self.n = n
        self.parents = [-1] * n

    def find(self, x):
        if self.parents[x] < 0:
            return x
        else:
            self.parents[x] = self.find(self.parents[x])
            return self.parents[x]

    def union(self, x, y):
        x = self.find(x)
        y = self.find(y)

        if x == y:
            return

        if self.parents[x] > self.parents[y]:
            x, y = y, x

        self.parents[x] += self.parents[y]
        self.parents[y] = x

    def size(self, x):
        return -self.parents[self.find(x)]

    def same(self, x, y):
        return self.find(x) == self.find(y)

    def members(self, x):
        root = self.find(x)
        return [i for i in range(self.n) if self.find(i) == root]

    def roots(self):
        return [i for i, x in enumerate(self.parents) if x < 0]

    def group_count(self):
        return len(self.roots())

    def all_group_members(self):
        group_members = defaultdict(list)
        for member in range(self.n):
            group_members[self.find(member)].append(member)
        return group_members


def calc_score(done):
    return np.sum(done)*9


def get_union_find(A, limit, diff=1):
    uf = UnionFind(N*N)
    for y in range(N):
        for x in range(N):
            if limit-diff <= A[y][x] <= limit:  # マスの大きさがi-1以上のものを連結する
                index0 = y*N + x  # 盤面のindex
                for dy, dx in [[-1, 0], [1, 0], [0, -1], [0, 1]]:
                    # 上下左右を探索
                    ny, nx = y+dy, x+dx
                    if 0 <= ny < N and 0 <= nx < N and limit - diff <= A[ny][nx] <= limit:
                        index1 = ny*N + nx
                        uf.union(index0, index1)  # 連結
    return uf


def make_convex_m(A, dis):
    size = dis*2 + 1
    convex_m = {(i, j): [size*size-4, 0] for i in range(N) for j in range(N)}
    for i in range(N):
        for j in range(N):
            convex_m[(i, j)][1] = np.sum(
                A[max(i-dis, 0):min(i+dis+1, N), max(j-dis, 0):min(j+dis+1, N)])
            # マンハッタン距離がdis以下にならないものはのぞいておく
            if 0 <= i - dis < N and 0 <= j - dis < N:
                convex_m[(i, j)][1] -= A[i-dis][j-dis]
            if 0 <= i + dis < N and 0 <= j + dis < N:
                convex_m[(i, j)][1] -= A[i+dis][j+dis]
            if 0 <= i - dis < N and 0 <= j + dis < N:
                convex_m[(i, j)][1] -= A[i-dis][j+dis]
            if 0 <= i + dis < N and 0 <= j - dis < N:
                convex_m[(i, j)][1] -= A[i+dis][j-dis]
    for j in range(dis):
        for i in range(N):
            convex_m[(j, i)][0] -= dis
            convex_m[(N-1-j, i)][0] -= dis

    for j in range(dis):
        for i in range(N):
            convex_m[(i, j)][0] -= j+1
            convex_m[(i, N-1-j)][0] -= j+1
    return convex_m


def reconstruct_ans(ans, raw_A, A):
    # 得点が得られない操作を消す関数

    target = set()
    uf = get_union_find(A, 9, 0)
    all_member = uf.all_group_members()
    member_list = [all_member[i] for i in all_member]
    for member in member_list:
        # 9以上memberがいれば得点
        if len(member) >= 9:
            continue
        else:
            for i in member:
                y, x = i//N, i % N
                target.add((y, x))
    re_ans = []  # 訂正した答え

    for i, y, x in ans:
        if i == 2:
            re_ans.append([i, y, x])
            continue
        if (y, x) in target:
            done[y][x] = False
            continue

        re_ans.append([i, y, x])
    return re_ans, done


def serach_different_loc(q, convex_m, done):
    # 今探している以外のところで効率が良い場所をheapqに追加する関数
    lis = []
    for i in range(N):
        for j in range(N):
            if done[i][j] is False:
                lis.append([convex_m[(i, j)], i, j])
    lis = sorted(lis, reverse=True)
    for _, i, j in lis[:20]:
        heapq.heappush(q, [-10**9, i, j])
    return q


# 入力
N, M = map(int, input().split())
B = [[*map(int, input().split())] for _ in range(N)]
C = np.array([np.array(i) for i in B])

bestscore = 0
bestans = -1
count = 1
# parameter

best_d = 1
best_field = -1

for diss in [1]:
    t = time.time()
    if t - start > 1.5:
        break
    for k in [239, 241, 242, 240, 244, 239, 245, 243, 277, 276, 238]:
        t = time.time()
        if t - start > 1.5:
            break
        for l in [1600, 1700, 1800, 1900, 2000]:
            t = time.time()
            if t - start > 1.5:
                break

            A = copy.deepcopy(C)
            dis = diss
            size = dis*2 + 1
            done = [[False]*N for _ in range(N)]
            ans = []
            for num in range(2, 10):
                uf = get_union_find(A, num, diff=0)
                all_group = uf.all_group_members()
                members = [[len(all_group[l]), all_group[l]]
                           for l in all_group if len(all_group[l]) >= num]
                for _, member in members:
                    for i in member:
                        y, x = i // N, i % N
                        done[y][x] = True
                        A[y][x] = 0
                    ans.append([2, y, x])
            c = 1
            # 畳み込み
            convex_m = make_convex_m(A, 1)
            kouho = []
            for i in range(N):
                for j in range(N):
                    kouho.append(
                        [convex_m[(i, j)][1]/convex_m[(i, j)][0], i, j])
            kouho.sort(reverse=True)
            q = []
            for i in range(k):
                heapq.heappush(q, [-10**9, kouho[i][1], kouho[i][2]])
            for i in range(9, 2, -1):
                re_flg = True
                while (len(q)):
                    # # 別のところを探してみる
                    # if len(ans) > 2000 and flg:
                    #     q = serach_different_loc(q, convex_m, done)
                    #     flg = False
                    # M= 2460を超えたあたりで、点数にならないものを消しておく
                    if len(ans) >= 2300:
                        raw_A = copy.deepcopy(C)
                        ans, done = reconstruct_ans(ans, raw_A, A)
                        break
                    _, y, x = heapq.heappop(q)
                    if done[y][x]:
                        continue
                    diff = abs(A[y][x] - 9)
                    while (diff > 0):
                        ans.append([1, y, x])
                        diff -= 1

                    A[y][x] = 9
                    done[y][x] = True
                    for dy, dx in [[-1, 0], [1, 0], [0, 1], [0, -1]]:
                        ny, nx = y + dy, x + dx
                        if 0 <= ny < N and 0 <= nx < N and done[ny][nx] is False:
                            if A[ny][nx] == 9:
                                heapq.heappush(q, [-10**9, ny, nx])
                            elif A[ny][nx] <= 3:
                                continue
                            else:
                                heapq.heappush(
                                    q, [-A[ny][nx] - 0.1*(c/l)*convex_m[(ny, nx)][1]/(convex_m[(ny, nx)][0]), ny, nx])
                                c += 1
                score = calc_score(done)
                if bestscore < score:
                    best_d = dis
                    bestans = ans
                    bestscore = score
                    best_field = copy.deepcopy(A)
                    best_done = done
_best_ans = copy.deepcopy(bestans)
uf = UnionFind(N*N)
for i in range(N):
    for j in range(N):
        if best_field[i][j] == 9:
            index0 = i*N + j
            for di, dj in [[-1, 0], [1, 0], [0, 1], [0, -1]]:
                ni, nj = i+di, j+dj
                if 0 <= ni < N and 0 <= nj < N and best_field[ni][nj] == 9:
                    index1 = ni*N + nj
                    uf.union(index0, index1)

all_group = uf.all_group_members()
syuukaku_list = []
for i in all_group:
    if len(all_group[i]) >= 9:
        m = all_group[i][0]
        syuukaku_list.append([m//N, m % N])


# field_の見直し
A = copy.deepcopy(C)

for i in range(len(bestans)):
    i, y, x = bestans[i]
    if i == 2:
        continue
    A[y][x] += 1

uf = UnionFind(N*N)
for i in range(N):
    for j in range(N):
        if A[i][j] == 9:
            index0 = i*N + j
            for di, dj in [[-1, 0], [1, 0], [0, 1], [0, -1]]:
                ni, nj = i+di, j+dj
                if 0 <= ni < N and 0 <= nj < N and A[ni][nj] == 9:
                    index1 = ni*N + nj
                    uf.union(index0, index1)
all_group = uf.all_group_members()
best_syuukaku_list = []
for i in all_group:
    if len(all_group[i]) >= 9:
        m = all_group[i][0]
        best_syuukaku_list.append([m//N, m % N])

_best_ans = _best_ans[:M-len(best_syuukaku_list)]
for y, x in best_syuukaku_list:
    _best_ans.append([2, y, x])

A = copy.deepcopy(C)
done = [[False]*N for _ in range(N)]
for i, y, x in _best_ans:
    if i == 1:
        A[y][x] += 1
    done[y][x] = True
for val in range(2, 10):
    uf = get_union_find(A, val, 0)
    all_group = uf.all_group_members()
    groups = [all_group[i] for i in all_group if len(all_group[i]) >= val]
    for member in groups:
        for i in member:
            y, x = i//N, i % N
            done[y][x] = True

# 5,6埋め
q = []
for val in range(8, 1, -1):
    uf = get_union_find(A, val, 1)
    all_group = uf.all_group_members()
    members = [all_group[j] for j in all_group if len(all_group[j]) >= val]
    for member in members:
        points = [A[i//N][i % N] for i in member]
        if len(set(points)) == 1:
            continue
        count = 0
        for j in member:
            y, x = j//N, j % N
            if done[y][x] is False:
                count += 1

        if count >= val:
            score = 0
            cost = 0
            count = 0
            for j in member:
                y, x = j//N, j % N
                if done[y][x]:
                    continue
                count += 1
                if A[y][x] == val:
                    continue
                cost += 1
                score += val
            if cost == 0:
                continue
            if count >= val:
                heapq.heappush(q, [-score/cost, val, member])

while (len(q)):
    if len(_best_ans) >= M:
        break
    _, _val, member = heapq.heappop(q)
    count = 0
    for i in member:
        y, x = i//N, i % N
        if done[y][x] is False:
            count += 1
    if count < val:
        continue
    for i in member:
        y, x = i//N, i % N
        if A[y][x] == _val or done[y][x]:
            continue
        y0, x0 = y, x
        for _ in range(_val - A[y][x]):
            _best_ans.append([1, y, x])
        done[y][x] = True
    _best_ans.append([2, y0, x0])
_best_ans = _best_ans[:M]
for j, i in enumerate(_best_ans):
    print(*i)
