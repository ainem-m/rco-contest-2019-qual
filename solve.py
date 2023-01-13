N, M = map(int, input().split())
A = [[int(i) for i in input().split()] for _ in range(N)]


output = []

for i in range(50):
    if A[i][0] < 9:
        time = 9 - A[i][0]
        for _ in range(time):
            output.append(f"1 {i} 0")
        A[i][0] = 9
        M -= time
    if A[0][i] < 9:
        time = 9 - A[0][i]
        for _ in range(time):
            output.append(f"1 0 {i}")
        A[0][i] = 9
        M -= time

output.append("2 0 0")
print(*output, sep="\n")
