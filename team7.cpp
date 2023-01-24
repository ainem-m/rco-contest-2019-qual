#pragma GCC optimize("Ofast")
#include <iostream>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <cctype>
#include <sstream>
#include <vector>
#include <stack>
#include <deque>
#include <queue>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <chrono>
#include <random>

using namespace std;

using ll = long long;
using P = pair<int, int>;
template <class T>
using V = vector<T>;
template <class T>
using VV = V<V<T>>;
template <class T, class U>
bool chmin(T &t, const U &u)
{
    if (t > u)
    {
        t = u;
        return 1;
    }
    else
        return 0;
}
template <class T, class U>
bool chmax(T &t, const U &u)
{
    if (t < u)
    {
        t = u;
        return 1;
    }
    else
        return 0;
}

#define REP(i, n) for (int i = 0; i < int(n); i++)
#define FOR(i, a, b) for (int i = int(a); i <= int(b); i++)
#define FORD(i, a, b) for (int i = int(a); i >= int(b); i--)
#define MP make_pair
#define SZ(x) int(x.size())
#define ALL(x) x.begin(), x.end()
#define INF 10001000
#define PI 3.14159265358979323846
#define endl "\n"

chrono::system_clock::time_point t_start;
const double TIME_LIMIT1 = 1900, TIME_LIMIT = 1200;
double last_update = 0, t_diff;
double start_temp, end_temp;

mt19937 rng;
using uni_int = uniform_int_distribution<>;
using uni_real = uniform_real_distribution<>;
using u64 = uint64_t;
using u32 = uint32_t;

V<ll> y_vec = {0, 1, 0, -1};
V<ll> x_vec = {-1, 0, 1, 0};

// 時間計測
void get_time()
{
    auto t_now = chrono::system_clock::now();
    t_diff = chrono::duration_cast<chrono::milliseconds>(t_now - t_start).count();
}

// 以下解答コード//
const int MAX_VALUE = 9;
const int MID_VALUE = 7;
const int MIN_HARVEST = 200;
int N, M;
int cnt = 0;
V<int> diff;

//(x,y)を1次元に変換
int Pos(int x, int y)
{
    return x * N + y;
}

int Dtoi(int pre, int nex)
{
    if (pre / N == nex / N)
    {
        return (pre < nex ? 1 : 3);
    }
    return (pre < nex ? 2 : 0);
}

// 区画クラス
class Cell
{
public:
    // 区画の品質
    int value = 0;

    // 区画の初期品質
    int initValue = 0;

    // 収穫済みのフラグ（最後に収穫するなら不要かも）
    bool harvestFlag = false;

    // 周囲4方位に存在する区画の集合（毎回4方位が到達可能かを判別する手間を省く(bowowforeachさんのツイートで学んだ)）
    V<int> around4;

    Cell(){};

    void setValue(int value)
    {
        this->value = value;
        initValue = value;
    }

    // num回区画に手入れ
    void care(int num)
    {
        value += num;
    }

    // num回区画を破壊
    void decare(int num)
    {
        value -= num;
    }

    // 区画に何回手入れをしたか
    int careDiff()
    {
        return value - initValue;
    }

    bool isHarvested()
    {
        return harvestFlag;
    }

    void harvest()
    {
        harvestFlag = true;
    }
};

// 初期盤面保存用(グリッドを1次元配列で持つ)
V<Cell> initialGrid;

// 操作を表現するクラス
class Action
{
public:
    // 手入れ数を管理する長さ2500の文字列 cares[i]=(i番目の区画に手入れした数)
    string cares = "";

    // これまでの累計操作数
    int numOperation = 0;

    // 操作によって盤面で獲得できるスコア
    int score = 0;

    // 操作の価値（ソートするために保持）
    int value = 0;

    // 最初は手入れしないので0が2500続く
    Action()
    {
        REP(i, N * N)
        cares += '0';
    }

    int getCare(int pos)
    {
        return cares[pos] - '0';
    }

    void setCare(int pos, int care)
    {
        cares[pos] = char(care + '0');
    }

    int leftTurn()
    {
        return M - numOperation;
    }

    void useTurn(int turn)
    {
        numOperation += turn;
    }

    void setScore(int score)
    {
        this->score = score;
    }

    void applyDiff(int scoreDiff)
    {
        score += scoreDiff;
    }

    void setValue()
    {
        value = score * M + numOperation;
    }
};

bool operator<(const Action &l, const Action &r)
{
    return l.value < r.value;
}

class Component
{
public:
    int pos = 0;
    int value = 0;
    int num = 0;
    int cost = 0;

    Component(){};
    Component(int pos, int value, int num, int cost) : pos(pos), value(value), num(num), cost(cost){};

    bool canHarvest()
    {
        return num >= value / num;
    }
};

bool operator<(const Component &l, const Component &r)
{
    if (l.value != r.value)
        return l.value > r.value;
    if (l.cost != r.cost)
        return l.cost < r.cost;
    return l.num > r.num;
}

// 領域（盤面の一部）クラス
class Domain
{
public:
    int pos;
    int care;
    int target;
    double value;
    string route = "";
    bool fresh = false;
    // 初期位置からの上下左右の移動を0~3の文字列で表現

    Domain(int pos, int care, int target, double value)
        : pos(pos), care(care), target(target), value(value){};

    Domain(int pos, int care, int target, double value, string route)
        : pos(pos), care(care), target(target), value(value), route(route){};

    Domain(int pos, int care, int target, double value, string route, bool fresh)
        : pos(pos), care(care), target(target), value(value), route(route), fresh(fresh){};
};

bool operator<(const Domain &l, const Domain &r)
{
    // if (l.fresh != r.fresh)return l.fresh > r.fresh;
    if (l.value != r.value)
        return l.value < r.value;
    if (l.care != r.care)
        return l.care < r.care;
    return l.pos < r.pos;
}

// 状態(盤面)クラス
class State
{
public:
    V<Cell> grid;

    int usedTurn = 0;

    // 何回目のスコア計算かを保持
    int numVisit = 0;

    // 盤面の得点になる連結成分の数
    int numValuable = 0;

    // スコア計算のdfsで使用する。区画に訪れているかを管理する
    // intで管理することで毎回falseに初期化する必要がなくなる（tomerunさんのツイートで学んだ）
    V<int> visited;

    // スコア計算の際に使用する入れ物
    V<int> hangout;

    State()
    {
        visited = V<int>(N * N);
    };

    void setGrid(V<Cell> grid)
    {
        this->grid = grid;
    }

    // 操作を盤面に反映(全体更新)
    void perform(Action &action)
    {
        REP(pos, N * N)
        {
            // 価値7以上の野菜は確定で9まで手入れする
            if (grid[pos].initValue >= MID_VALUE)
                continue;
            grid[pos].care(action.getCare(pos));
            usedTurn += action.getCare(pos);
        }
    }

    // 操作を盤面に反映(1点更新)
    void perform(int pos, int care)
    {
        grid[pos].care(care);
        usedTurn += care;
    }

    void perform(Domain &domain)
    {
        int tpos = domain.pos;
        int care = domain.target - value(tpos);
        perform(tpos, care);
        for (char dir : domain.route)
        {
            tpos += diff[dir - '0'];
            care = domain.target - value(tpos);
            perform(tpos, care);
        }
    }

    // 操作を盤面から逆戻し(全体更新)
    void cancel(Action &action)
    {
        REP(pos, N * N)
        {
            // 価値7以上の野菜は確定で9まで手入れする
            if (grid[pos].initValue >= MID_VALUE)
                continue;
            grid[pos].decare(action.getCare(pos));
            usedTurn -= action.getCare(pos);
        }
    }

    // 操作を盤面から逆戻し(1点更新)
    void cancel(int pos, int care)
    {
        grid[pos].decare(care);
        usedTurn -= care;
    }

    void output()
    {
        REP(pos, N * N)
        {
            if (careDiff(pos) == 0)
                continue;
            REP(k, careDiff(pos))
            {
                cout << "1 " << pos / N << " " << pos % N << endl;
            }
        }
        int leftTurn = M - usedTurn;
        V<Component> positions;
        components(positions, true);
        REP(i, min(leftTurn, SZ(positions)))
        {
            int pos = positions[i].pos;
            cout << "2 " << pos / N << " " << pos % N << endl;
        }
        cerr << "harvest=" << min(leftTurn, SZ(positions)) << endl;
    }

    // 素の盤面評価。(O(N^2))
    int getScore(int leftTurn)
    {
        hangout = V<int>();
        numVisit++;
        numValuable = 0;
        REP(pos, N * N)
        {
            if (isVisited(pos))
                continue;
            int numComp = 0;
            dfs(pos, numComp);
            if (numComp > 1)
                numValuable += numComp;
            if (numComp < value(pos))
                continue;
            hangout.push_back(numComp * value(pos));
        }
        sort(ALL(hangout), greater<int>());
        int score = 0;
        REP(i, min(SZ(hangout), leftTurn))
        {
            score += hangout[i];
        }
        return score;
    }

    int getScore()
    {
        return getScore(M - usedTurn);
    }

    // 連結成分が品質未満でも一部点数をあげる優しい評価
    int getSoftScore(int leftTurn)
    {
        hangout = V<int>();
        numVisit++;
        numValuable = 0;
        REP(pos, N * N)
        {
            if (isVisited(pos))
                continue;
            int numComp = 0;
            int numCost = 0;
            dfs(pos, numComp, numCost);
            if (numComp > 3 && numComp * value(pos) > 10)
                numValuable += numComp * 7 / 10;
            if (numComp < value(pos))
            {
                hangout.push_back(numComp * value(pos) * 7 / 10 + 2 * numComp - numCost * 7 / 10);
                continue;
            }
            hangout.push_back(numComp * value(pos) + 2 * numComp - numCost * 7 / 10);
        }
        sort(ALL(hangout), greater<int>());
        int score = 0;
        REP(i, min(SZ(hangout), leftTurn))
        {
            score += hangout[i];
        }
        return score + numValuable;
    }

    // 収穫位置と連結成分のスコアを取得
    void components(V<Component> &positions, bool output = false)
    {
        numVisit++;
        REP(pos, N * N)
        {
            if (isVisited(pos))
                continue;
            int numComp = 0;
            int cost = 0;
            dfs(pos, numComp, cost);
            if (output && numComp < value(pos))
                continue;
            positions.push_back(Component(pos, numComp * value(pos), numComp, cost));
        }
        sort(ALL(positions));
    }

    void dfs(int pos, int &numComp)
    {
        numComp++;
        visit(pos);
        for (auto npos : around4(pos))
        {
            if (isVisited(npos))
                continue;
            if (value(pos) != value(npos))
                continue;
            dfs(npos, numComp);
        }
    }

    void dfs(int pos, int &numComp, int &cost)
    {
        numComp++;
        cost += careDiff(pos);
        visit(pos);
        for (auto npos : around4(pos))
        {
            if (isVisited(npos))
                continue;
            if (value(pos) != value(npos))
                continue;
            dfs(npos, numComp, cost);
        }
    }

    // サーチで生まれた意味のない手入れを回収して再配置
    void aftercare()
    {
        get_time();
        int numHarvest = setHarvest();
        redistribute(numHarvest);
    }

    // 収穫が確定した区画に収穫フラグを立てる
    int setHarvest()
    {
        int leftTurn = M - usedTurn;
        V<Component> positions;
        components(positions);
        numVisit++;
        int numHarvet = 0;
        REP(i, min(leftTurn, SZ(positions)))
        {
            int val = positions[i].value;
            int pos = positions[i].pos;
            int num = positions[i].num;
            if (val < 12)
                continue;
            if (num < val / num)
                continue;
            harvestDfs(pos);
            numHarvet++;
        }
        return numHarvet;
    }

    void harvestDfs(int pos)
    {
        visit(pos);
        harvest(pos);
        for (auto npos : around4(pos))
        {
            if (isVisited(npos))
                continue;
            if (value(pos) != value(npos))
                continue;
            harvestDfs(npos);
        }
    }

    void redistribute(int numHarvest)
    {
        priority_queue<Domain> pq;
        REP(pos, N * N)
        {
            if (isHarvested(pos))
                continue;
            if (careDiff(pos) > 0)
                cancel(pos, careDiff(pos));
        }
        REP(pos, N * N)
        {
            if (isHarvested(pos))
                continue;
            for (auto npos : around4(pos))
            {
                if (!isHarvested(npos))
                    continue;
                expandDomain(pos, value(npos), pq);
            }
        }
        REP(pos, N * N)
        {
            if (isHarvested(pos))
                continue;
            if (t_diff > TIME_LIMIT1)
                break;
            if (value(pos) <= 2)
                continue;
            createDomain(pos, pq);
        }
        while (!pq.empty())
        {
            Domain domain = pq.top();
            pq.pop();
            if (isHarvested(domain))
                continue;
            if (domain.fresh)
            {
                if (usedTurn + numHarvest + 1 + domain.care > M)
                    continue;
            }
            else
            {
                if (usedTurn + numHarvest + domain.care > M)
                    continue;
            }
            perform(domain);
            harvest(domain);
            if (domain.fresh)
                numHarvest++;
            int tpos = domain.pos;
            for (auto npos : around4(tpos))
            {
                if (isHarvested(npos))
                    continue;
                if (t_diff < TIME_LIMIT1)
                    expandDomain(npos, value(tpos), pq);
            }
            for (char dir : domain.route)
            {
                tpos += diff[dir - '0'];
                for (auto npos : around4(tpos))
                {
                    if (isHarvested(npos))
                        continue;
                    if (t_diff < TIME_LIMIT1)
                        expandDomain(npos, value(tpos), pq);
                }
            }
            get_time();
            if (t_diff > TIME_LIMIT1)
            {
                break;
            }
        }
        // cerr << minValue << endl;
    }

    void expandDomain(int pos, int targetValue, priority_queue<Domain> &pq)
    {
        if (targetValue < value(pos))
            return;
        numVisit++;
        string route = "";
        int sumValue = 0, sumCost = 0;
        expandDfs(pos, pos, sumValue, sumCost, targetValue, 1, route, pq);
    }

    void expandDfs(int pos, int &initPos, int &sumValue, int &sumCost, int &targetValue, int depth,
                   string &route, priority_queue<Domain> &pq)
    {
        if (depth > 6)
            return;
        sumValue += value(pos);
        sumCost += targetValue - value(pos);
        visit(pos);
        if (sumCost / depth <= 3 && double(sumValue) / double(sumCost) >= 2.0)
            pq.push(Domain(initPos, sumCost, targetValue, double(sumValue) / double(sumCost), route, false));
        for (auto npos : around4(pos))
        {
            if (isHarvested(npos))
                continue;
            if (isVisited(npos))
                continue;
            if (targetValue < value(npos))
                continue;
            route += char('0' + Dtoi(pos, npos));
            expandDfs(npos, initPos, sumValue, sumCost, targetValue, depth + 1, route, pq);
            route.pop_back();
        }
        unvisit(pos);
        sumCost -= targetValue - value(pos);
        sumValue -= value(pos);
    }

    void createDomain(int pos, priority_queue<Domain> &pq)
    {
        FOR(targetValue, max(3, value(pos)), min(value(pos) + 3, 9))
        {
            if (t_diff > TIME_LIMIT1)
                break;
            numVisit++;
            int sumValue = 0, sumCost = 0;
            string route = "";
            createDfs(pos, pos, sumValue, sumCost, targetValue, 1, 1, route, pq);
            get_time();
        }
    }

    void createDfs(int pos, int &initPos, int &sumValue, int &sumCost, int &targetValue, int depth, int cnt,
                   string &route, priority_queue<Domain> &pq)
    {
        if (depth > 9)
            return;
        if (cnt >= 1000)
            return;
        sumValue += value(pos);
        sumCost += targetValue - value(pos);
        visit(pos);
        cnt++;
        if (depth >= targetValue && sumCost / depth <= 3 && double(sumValue) / double(sumCost + 0.1) >= 2.0)
            pq.push(Domain(initPos, sumCost, targetValue, double(sumValue) / double(sumCost), route, true));
        for (auto npos : around4(pos))
        {
            if (isHarvested(npos))
                continue;
            if (isVisited(npos))
                continue;
            if (targetValue < value(npos))
                continue;
            bool ok = false;
            for (auto nnpos : around4(npos))
            {
                if (value(nnpos) == targetValue && isHarvested(nnpos))
                {
                    ok = true;
                    break;
                }
            }
            if (ok)
                continue;
            route += char('0' + Dtoi(pos, npos));
            createDfs(npos, initPos, sumValue, sumCost, targetValue, depth + 1, cnt, route, pq);
            route.pop_back();
        }
        unvisit(pos);
        sumCost -= targetValue - value(pos);
        sumValue -= value(pos);
    }

    bool isVisited(int pos)
    {
        return visited[pos] == numVisit;
    }

    void visit(int pos)
    {
        visited[pos] = numVisit;
    }

    void unvisit(int pos)
    {
        visited[pos] = numVisit - 1;
    }

    bool isHarvested(int pos)
    {
        return grid[pos].isHarvested();
    }

    bool isHarvested(Domain &domain)
    {
        int tpos = domain.pos;
        if (isHarvested(tpos))
            return true;
        for (char dir : domain.route)
        {
            tpos += diff[dir - '0'];
            if (isHarvested(tpos))
                return true;
        }
        return false;
    }

    void harvest(int pos)
    {
        grid[pos].harvest();
    }

    void harvest(Domain &domain)
    {
        int tpos = domain.pos;
        harvest(tpos);
        for (char dir : domain.route)
        {
            tpos += diff[dir - '0'];
            harvest(tpos);
        }
    }

    int value(int pos)
    {
        return grid[pos].value;
    }

    int careDiff(int pos)
    {
        return grid[pos].careDiff();
    }

    V<int> &around4(int pos)
    {
        return grid[pos].around4;
    }

    void print()
    {
        REP(x, N)
        {
            REP(y, N)
            {
                cerr << value(Pos(x, y));
            }
            cerr << endl;
        }
    }
};

// chokudaiサーチ(時間いっぱい回す)thunderさんの実装を参考にしています。
// 盤面はpriority_queueに入れず、毎回初期盤面から再現する(&初期状態に戻す)
// 一つずつ区画の手入れ数を決める
void chokudaiSearch(State &state, V<Action> &actions, int beamWidth)
{
    Action initAction;
    REP(pos, N * N)
    {
        if (state.value(pos) == MAX_VALUE)
            continue;
        if (state.value(pos) >= MID_VALUE)
        {
            int care = MAX_VALUE - state.value(pos);
            initAction.setCare(pos, care);
            initAction.useTurn(care);
            state.perform(pos, care);
        }
    }

    V<int> order;
    REP(i, N * N)
    {
        if (state.value(i) <= 4)
            continue;
        if (state.value(i) == 9)
            continue;
        order.push_back(i);
    }

    // 序盤で選択肢が爆発しないように価値が大きいものから選択するためのソート
    sort(ALL(order), [&state](const int &l, const int &r)
         { return state.grid[l].initValue > state.grid[r].initValue; });

    int beamDepth = SZ(order);
    auto beam = V<priority_queue<Action>>(beamDepth + 1);
    REP(t, beamDepth + 1)
    {
        beam[t] = priority_queue<Action>();
    }

    initAction.setScore(state.getSoftScore(INF));
    initAction.setValue();
    beam[0].push(initAction);

    while (t_diff < TIME_LIMIT)
    {
        REP(depth, beamDepth)
        {
            auto &nowBeam = beam[depth];
            auto &nextBeam = beam[depth + 1];
            int pos = order[depth];

            // ビームの中身が多すぎるときに減らす(やらないとメモリが増えて時間管理できなくなる)
            int maxLoad = 200;
            if (SZ(nowBeam) > maxLoad)
            {
                priority_queue<Action> pool;
                // 最大量の半分まで減らす
                REP(i, maxLoad / 2)
                {
                    pool.push(nowBeam.top());
                    nowBeam.pop();
                }
                nowBeam.swap(pool);
            }

            REP(i, beamWidth)
            {
                if (nowBeam.empty())
                    break;

                Action nowAction = nowBeam.top();
                nowBeam.pop();
                int leftTurn = nowAction.leftTurn();

                // 盤面に操作を反映
                state.perform(nowAction);

                FOR(care, 0, max(0, min({MAX_VALUE - state.value(pos), leftTurn - MIN_HARVEST})))
                {
                    // 手入れが0なら状態は変化しないのでスコア計算をスキップ
                    if (care == 0)
                    {
                        nextBeam.push(nowAction);
                        continue;
                    }
                    Action newAction = nowAction;
                    newAction.setCare(pos, care);
                    newAction.useTurn(care);
                    state.perform(pos, care);
                    newAction.setScore(state.getSoftScore(leftTurn - care));
                    state.cancel(pos, care);
                    newAction.setValue();
                    nextBeam.push(newAction);
                }
                // 盤面から操作をキャンセル（初期盤面に戻る）
                state.cancel(nowAction);
            }
            get_time();
            if (t_diff > TIME_LIMIT)
                break;
        }
        cnt++;
    }
    int num = 1;
    while (num)
    {
        Action action = beam[beamDepth].top();
        beam[beamDepth].pop();
        actions.push_back(action);
        if (beam[beamDepth].empty())
            break;
        num--;
    }
}

void input()
{
    cin >> N >> M;
    initialGrid = V<Cell>(N * N);
    REP(i, N * N)
    {
        int value;
        cin >> value;
        initialGrid[i].setValue(value);
    }
    diff = {-N, 1, N, -1};
}

// グリッドの初期化（各マスについて4方位の隣接区画を前計算)
void initialize()
{
    REP(x, N)
    REP(y, N)
    {
        auto &cell = initialGrid[Pos(x, y)];
        REP(d, 4)
        {
            int nx = x + x_vec[d];
            int ny = y + y_vec[d];
            if (nx < 0 || nx >= N || ny < 0 || ny >= N)
                continue;
            cell.around4.push_back(Pos(nx, ny));
        }
        shuffle(ALL(cell.around4), rng);
    }
}

int solve()
{
    State state;
    state.setGrid(initialGrid);

    V<Action> actions;
    chokudaiSearch(state, actions, 1);
    Action bestAction = actions[0];
    state.perform(bestAction);
    state.aftercare();
    state.output();
    // state.print();
    return state.getScore();
}

signed main()
{
    cin.tie(0);
    ios::sync_with_stdio(false);
    t_start = chrono::system_clock::now();
    get_time();

    input();
    initialize();
    int score = solve();

    get_time();
    cerr << "time=" << int(t_diff) << " ms" << endl;
    cerr << "score=" << score << endl;
    cerr << "cnt=" << cnt << endl;
    return 0;
}
