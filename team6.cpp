#include <bits/stdc++.h>
#define rep(i, n) for (int i = 0; i < (n); i++)
using namespace std;
using ll = long long;

int N, M;
typedef pair<int, pair<int, int>> P;
vector<vector<int>> farm(50, vector<int>(50));

vector<pair<int, int>> sima_vec;
int sima_ct = 0;

void sima_dfs(int x, int y, int &num, int step, int ct, double &score, vector<pair<int, int>> vec, set<pair<int, int>> set)
{
    if (x < 0 || x > 49 || y < 0 || y > 49)
        return;
    if (farm[x][y] > num)
        return;
    if (set.count({x, y}))
        return;
    if (step > 10)
        return;

    set.insert({x, y});
    vec.push_back({x, y});
    ct += num - farm[x][y];
    double retscore = (double)(num * step) / (double)(ct + 1);
    if (retscore > score && step >= num)
    {
        score = retscore;
        sima_vec = vec;
        sima_ct = ct;
    }

    sima_dfs(x + 1, y, num, step + 1, ct, score, vec, set);
    sima_dfs(x - 1, y, num, step + 1, ct, score, vec, set);
    sima_dfs(x, y + 1, num, step + 1, ct, score, vec, set);
    sima_dfs(x, y - 1, num, step + 1, ct, score, vec, set);
}

int main()
{
    cin >> N >> M;
    vector<vector<int>> grow_farm(N, vector<int>(N, 0));
    vector<string> harvest; // 解答
    vector<string> grow;
    bool out_flg = false; // outputファイルのフラグ

    vector<pair<int, int>> farm_9;

    rep(i, N) rep(j, N)
    {
        cin >> farm[i][j];
        if (farm[i][j] == 9)
        {
            farm_9.push_back({i, j});
        }
    }

    rep(i, N) rep(j, N)
    {
        cin >> farm[i][j];
        if (farm[i][j] == 8)
        {
            farm_9.push_back({i, j});
        }
    }

    for (int i = 0; i < farm_9.size() - 1; i++)
    {
        int sx = farm_9[i].first, sy = farm_9[i].second;
        int gx = farm_9[i + 1].first, gy = farm_9[i + 1].second;

        // ダイクストラ準備
        vector<vector<int>> dks_cost(N, vector<int>(N, 1e9));
        vector<vector<char>> dks_root(N, vector<char>(N, 'x'));

        // スタートは０
        dks_cost[sx][sy] = 0;
        priority_queue<P, vector<P>, greater<P>> que;
        que.push({0, {sx, sy}});
        set<pair<int, int>> ck_set;

        // cerr << sx <<" " << sy << " :: " << gx << " " << gy << endl;

        while (que.size())
        {
            int gole_cost = dks_cost[gx][gy];
            auto d = que.top();
            int x = d.second.first, y = d.second.second;

            ck_set.insert({x, y});

            // cerr << x << " " << y << endl;
            // cerr << dks_cost[x][y] << endl;

            que.pop();
            // 上下左右チェック
            for (int i = -1; i <= 1; i++)
                for (int j = -1; j <= 1; j++)
                {
                    if (x + i < 0 || x + i > 49 || y + j < 0 || y + j > 49)
                        continue;
                    if (abs(i) + abs(j) != 1)
                        continue;
                    if (ck_set.count({x + i, y + j}))
                        continue;

                    int cost = dks_cost[x][y] + (9 - farm[x + i][y + j]);
                    if (cost < gole_cost && cost < dks_cost[x + i][y + j])
                    {
                        dks_cost[x + i][y + j] = cost;
                        que.push({cost, {x + i, y + j}});

                        if (i == -1)
                            dks_root[x + i][y + j] = 'd';
                        if (i == 1)
                            dks_root[x + i][y + j] = 'u';
                        if (j == -1)
                            dks_root[x + i][y + j] = 'r';
                        if (j == 1)
                            dks_root[x + i][y + j] = 'l';
                    }
                }
            // cerr << gole_cost << endl;
        }

        // 　経路を辿って9にする。
        if (M - dks_cost[gx][gy] < 0)
        {
            break;
        }

        M -= dks_cost[gx][gy];

        int rx = gx, ry = gy;
        // cerr << "start " <<  sx << " " << sy << endl;
        while (rx != sx || ry != sy)
        {
            // cerr << rx << " " << ry << endl;
            int num = 9 - farm[rx][ry];
            while (num--)
            {
                grow.push_back("1 " + to_string(rx) + " " + to_string(ry));
            }
            farm[rx][ry] = 9;
            // cerr << dks_root[rx][ry] << endl;
            if (dks_root[rx][ry] == 'u')
                rx--;
            else if (dks_root[rx][ry] == 'd')
                rx++;
            else if (dks_root[rx][ry] == 'r')
                ry++;
            else if (dks_root[rx][ry] == 'l')
                ry--;
        }
    }

    // 収穫
    harvest.push_back("2 " + to_string(farm_9[0].first) + " " + to_string(farm_9[0].second));
    M--;

    cerr << M << endl;

    // コスパいいところは刈り取る +0~+してみる。
    while (1)
    {
        bool flg = true;
        vector<tuple<double, int, int, int, set<pair<int, int>>>> tp;
        rep(i, N) rep(j, N) rep(ct, 4)
        {
            int snum = farm[i][j] + ct;
            if (snum >= 9)
                continue;
            if (M - ct < 0)
                continue;
            set<pair<int, int>> cospa_set;
            queue<pair<int, int>> que;
            que.push({i, j});

            while (que.size())
            {
                auto q = que.front();
                que.pop();
                int x = q.first, y = q.second;

                if (cospa_set.count({x, y}))
                    continue;
                cospa_set.insert({x, y});

                for (int s = -1; s <= 1; s++)
                    for (int t = -1; t <= 1; t++)
                    {
                        if (x + s < 0 || x + s > 49 || y + t < 0 || y + t > 49)
                            continue;
                        if (abs(s) + abs(t) != 1)
                            continue;

                        if (farm[x + s][y + t] == snum)
                        {
                            que.push({x + s, y + t});
                        }
                    }
            }
            double score = ((double)cospa_set.size() * (double)snum) / (double)(ct + 1);
            if (score > 2 && cospa_set.size() >= snum)
            {
                tp.push_back({score, i, j, ct, cospa_set});
            }
        }

        if (tp.size() > 0)
        {
            sort(tp.rbegin(), tp.rend());
            M -= (get<3>(tp[0]) + 1);
            int sx = get<1>(tp[0]), sy = get<2>(tp[0]);
            for (auto tsgen : get<4>(tp[0]))
            {
                int x = tsgen.first, y = tsgen.second;
                farm[x][y] = 10;
                flg = false;
            }

            rep(p, get<3>(tp[0]))
            {
                grow.push_back("1 " + to_string(sx) + " " + to_string(sy));
            }
            harvest.push_back("2 " + to_string(sx) + " " + to_string(sy));
        }
        if (flg)
            break;
    }
    // cerr << M << endl;
    // M = 2500 - grow.size() - harvest.size();
    cerr << M << endl;

    // ７や６を延ばしてみる？
    vector<pair<int, int>> farm_7, farm_6;
    rep(i, N) rep(j, N)
    {
        if (farm[i][j] == 7)
            farm_7.push_back({i, j});
        if (farm[i][j] == 6)
            farm_6.push_back({i, j});
    }

    // ７チェック
    for (auto sima7 : farm_7)
    {
        int x = sima7.first, y = sima7.second;
        vector<pair<int, int>> vec;
        set<pair<int, int>> set;
        int num7 = 7;
        double score7 = 7;
        sima_ct = 0;
        sima_vec.clear();
        sima_dfs(x, y, num7, 1, 0, score7, vec, set);
        if (score7 > 2 && M - (sima_ct + 1) > 0 && sima_vec.size() >= 7)
        {
            for (auto vp : sima_vec)
            {
                int s = vp.first, t = vp.second;
                int ctnum = 7 - farm[s][t];
                rep(k, ctnum)
                {
                    grow.push_back("1 " + to_string(s) + " " + to_string(t));
                }
                farm[s][t] = 10;
            }
            harvest.push_back("2 " + to_string(x) + " " + to_string(y));
        }
        M = 2500 - grow.size() - harvest.size();
    }

    // 6チェック
    for (auto sima6 : farm_6)
    {
        int x = sima6.first, y = sima6.second;
        vector<pair<int, int>> vec;
        set<pair<int, int>> set;
        int num6 = 6;
        double score6 = 6;
        sima_ct = 0;
        sima_vec.clear();
        sima_dfs(x, y, num6, 1, 0, score6, vec, set);
        if (score6 > 2 && M - (sima_ct + 1) > 0 && sima_vec.size() >= 6)
        {
            for (auto vp : sima_vec)
            {
                int s = vp.first, t = vp.second;
                int ctnum = 6 - farm[s][t];
                rep(k, ctnum)
                {
                    grow.push_back("1 " + to_string(s) + " " + to_string(t));
                }
                farm[s][t] = 10;
            }
            harvest.push_back("2 " + to_string(x) + " " + to_string(y));
        }
        M = 2500 - grow.size() - harvest.size();
    }
    cerr << M << endl;

    // BFS
    for (int hnum = 1; hnum <= 4; hnum++)
    {
        rep(i, N) rep(j, N)
        {
            if (farm[i][j] >= 8)
                continue;
            set<pair<int, int>> bfs_set;
            queue<pair<int, int>> bfs_que;
            bfs_que.push({i, j});
            int ct = 1, snum = farm[i][j];
            while (bfs_que.size())
            {
                auto que_top = bfs_que.front();
                int x = que_top.first, y = que_top.second;
                bfs_que.pop();
                if (bfs_set.count({x, y}))
                    continue;

                bfs_set.insert({x, y});
                for (int s = -1; s <= 1; s++)
                    for (int t = -1; t <= 1; t++)
                    {
                        if (x + s < 0 || x + s > 49 || y + t < 0 || y + t > 49)
                            continue;
                        if (abs(s) + abs(t) != 1)
                            continue;
                        if (farm[x + s][y + t] > snum)
                            continue;
                        if (farm[x + s][y + t] < snum - hnum)
                            continue;

                        if (M > snum - farm[x + s][y + t] + ct)
                        {
                            ct += snum - farm[x + s][y + t];
                            bfs_que.push({x + s, y + t});
                        }
                    }
            }

            // 採用すべきか
            if (bfs_set.size() < snum)
                continue;
            if (bfs_set.size() * snum > 2 * ct)
            {
                for (auto sgen : bfs_set)
                {
                    int x = sgen.first, y = sgen.second;
                    int gct = snum - farm[x][y];
                    while (gct--)
                    {
                        grow.push_back("1 " + to_string(x) + " " + to_string(y));
                    }
                    farm[x][y] = 10;
                }
                harvest.push_back("2 " + to_string(i) + " " + to_string(j));
            }
            M = 2500 - grow.size() - harvest.size();
        }
    }

    // 残り回数分9に近いところから大きくしていく
    int round_num = 8;
    // 操作可能なら繰り返す
    while (M - (9 - round_num) > 0)
    {
        bool flg = false;
        rep(x, N) rep(y, N)
        {
            if (M - (9 - round_num) < 1)
                break;
            if (farm[x][y] == round_num)
            {
                // 上下左右に9があれば9にする
                for (int i = -1; i <= 1; i++)
                    for (int j = -1; j <= 1; j++)
                    {
                        if (x + i < 0 || x + i > 49 || y + j < 0 || y + j > 49)
                            continue;
                        if (abs(i) + abs(j) != 1)
                            continue;

                        int ct = 9 - farm[x][y];
                        if (farm[x + i][y + j] == 9)
                        {
                            M -= ct;
                            rep(z, ct)
                            {
                                grow.push_back("1 " + to_string(x) + " " + to_string(y));
                            }
                            farm[x][y] = 9;
                            flg = true;
                            break;
                        }
                    }
            }
            if (flg == true && round_num < 8)
                break;
        }
        if (!flg)
            round_num--;
        else
            round_num = 8;
    }

    int ansline = 0;
    if (out_flg)
    {
        ofstream writing_file;
        string filename = "out.txt";
        writing_file.open(filename, std::ios::out);
        for (auto s : grow)
        {
            writing_file << s << endl;
            ansline++;
        }

        for (auto s : harvest)
        {
            if (ansline > 2500)
            {
                cerr << "over" << endl;
                break;
            }
            writing_file << s << endl;
            ansline++;
        }
        writing_file.close();
    }
    else
    {
        for (auto s : grow)
        {
            if (ansline > 2500)
            {
                cerr << "over" << endl;
                break;
            }
            cout << s << endl;
            ansline++;
        }
        for (auto s : harvest)
        {
            if (ansline > 2500)
            {
                cerr << "over" << endl;
                break;
            }
            cout << s << endl;
            ansline++;
        }
    }
}
