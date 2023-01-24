#include <iostream>
#include <vector>
#include <array>
#include <list>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include <cassert>
#include <climits>
#include <bitset>
#include <functional>
#include <iomanip>
#include <random>
#include <optional>
#include <chrono>

#define FOR_LT(i, beg, end) for (std::remove_const<decltype(end)>::type i = beg; i < end; i++)
#define FOR_LE(i, beg, end) for (std::remove_const<decltype(end)>::type i = beg; i <= end; i++)
#define FOR_DW(i, beg, end) for (std::remove_const<decltype(beg)>::type i = beg; end <= i; i--)
#define REP(n) for (std::remove_reference<std::remove_const<decltype(n)>::type>::type repeat_index = 0; repeat_index < n; repeat_index++)

using namespace std;

struct Point
{
    int x;
    int y;

    bool operator==(const Point &rhs) const
    {
        return this->x == rhs.x && this->y == rhs.y;
    }

    bool operator<(const Point &rhs) const
    {
        if (this->x != rhs.x)
        {
            return this->x < rhs.x;
        }
        return this->y < rhs.y;
    }
};

struct PointHash
{
    size_t operator()(const Point &point) const
    {
        return point.x * 50 + point.y;
    }
};

struct Board
{
    int cell[50][50];

    double average_filter(const Point &p) const
    {
        int sum = 0;
        int count = 0;
        constexpr Point dd[] = {{-1, 0}, {0, -1}, {0, 0}, {0, 1}, {1, 0}};
        for (const auto &d : dd)
        {
            const Point &np = {p.x + d.x, p.y + d.y};
            if (!this->is_in_board(np))
                continue;
            sum += cell[np.x][np.y];
            count++;
        }

        return (double)sum / count;
    }

    int max_filter(const Point &p) const
    {
        int val = 0;
        constexpr Point dd[] = {{-1, 0}, {0, -1}, {0, 0}, {0, 1}, {1, 0}};
        for (const auto &d : dd)
        {
            const Point &np = {p.x + d.x, p.y + d.y};
            if (!this->is_in_board(np))
                continue;
            val = max(val, cell[np.x][np.y]);
        }
        return val;
    }
    static bool is_in_board(const Point &p)
    {
        if (p.x < 0 || 50 <= p.x || p.y < 0 || 50 <= p.y)
            return false;
        return true;
    }
};

struct BoardDencity
{
    int dencity_map[10][10];
    int total_dencity = 0;
    static const int kAreaSize = 5;

    BoardDencity(const Board &board)
    {
        FOR_LT(i, 0, 10)
        {
            FOR_LT(j, 0, 10)
            {
                int xhead = i * 5;
                int yhead = j * 5;
                int dencity = 0;
                FOR_LT(k, 0, kAreaSize)
                {
                    FOR_LT(l, 0, kAreaSize)
                    {
                        int x = xhead + k;
                        int y = yhead + l;
                        dencity += board.cell[x][y];
                    }
                }
                // cerr << dencity << endl;
                this->dencity_map[i][j] = dencity * dencity;
                this->total_dencity += dencity * dencity;
            }
        }
    }

    Point sum_dencity_to_pos(int sum_dencity) const
    {
        assert(sum_dencity < this->total_dencity);
        // std::cerr << sum_dencity << " " << this->total_dencity << endl;
        int dencity = 0;
        FOR_LT(i, 0, 10)
        {
            FOR_LT(j, 0, 10)
            {
                dencity += this->dencity_map[i][j];
                if (sum_dencity < dencity)
                {
                    return {i * 5, j * 5};
                }
            }
        }
        assert(false);
        return {-1, -1};
    }
};

struct TakenGroup
{
    unordered_map<Point, int, PointHash> belonging;
    unordered_map<int, vector<Point>> belonging_point_list;
    unordered_map<int, int> belonging_to_score;
    multiset<int> score;
    int next_belonging_idx = 0;
    int cultivation_count = 0;
};

bool insert_search(const Point &p, int cell_score, int score, bool (&is_used)[50][50], queue<Point> &search)
{
    if (!Board::is_in_board(p))
        return false;
    if (is_used[p.x][p.y])
        return false;
    if (cell_score != score)
        return false;

    is_used[p.x][p.y] = true;
    search.push(p);
    return true;
}

optional<int> harvest(const Board &board, bool (&is_used)[50][50], const Point &p)
{
    if (is_used[p.x][p.y])
        return std::nullopt;

    int score = board.cell[p.x][p.y];
    int c = 0;
    queue<Point> search;
    search.push(p);
    c++;
    is_used[p.x][p.y] = true;
    while (!search.empty())
    {
        Point cp = search.front();
        search.pop();

        Point dp[4] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
        for (const auto &d : dp)
        {
            Point np = {cp.x + d.x, cp.y + d.y};
            if (insert_search(np, board.cell[np.x][np.y], score, is_used, search))
                c++;
        }
    }

    if (c < score)
        return std::nullopt;

    return c * board.cell[p.x][p.y];
}

vector<Point> execute_harvest(const Board &board, int max_count)
{
    bool is_used[50][50] = {};

    multimap<int, Point> harvest_list;
    FOR_LT(i, 0, 50)
    {
        FOR_LT(j, 0, 50)
        {
            auto harvest_result = harvest(board, is_used, {i, j});
            if (harvest_result)
            {
                harvest_list.insert({harvest_result.value(), {i, j}});
            }
        }
    }

    vector<Point> harvest_oplist;
    int harvest_count = 0;
    for (auto it = harvest_list.rbegin(); it != harvest_list.rend(); it++)
    {
        if (harvest_count == max_count)
            break;
        harvest_oplist.push_back(it->second);
        harvest_count++;
    }
    return harvest_oplist;
}

int evaluate(const Board &board, const Board &culltivation)
{
    Board value;
    int max_count = 2500;
    FOR_LT(i, 0, 50)
    {
        FOR_LT(j, 0, 50)
        {
            value.cell[i][j] = board.cell[i][j] + culltivation.cell[i][j];
            max_count -= culltivation.cell[i][j];
        }
    }
    if (max_count <= 0)
    {
        return 0;
    }

    bool is_used[50][50] = {};
    multiset<int> harvest_list;
    FOR_LT(i, 0, 50)
    {
        FOR_LT(j, 0, 50)
        {
            auto harvest_result = harvest(value, is_used, {i, j});
            if (harvest_result)
            {
                harvest_list.insert(harvest_result.value());
            }
        }
    }

    int score = 0;
    int harvest_count = 0;
    for (auto it = harvest_list.rbegin(); it != harvest_list.rend(); it++)
    {
        if (harvest_count == max_count)
            break;
        score += *it;
        harvest_count++;
    }
    return score;
}

vector<Point> conected_point(const Board &board, bool (&is_used)[50][50], const Point &p)
{
    if (is_used[p.x][p.y])
        return {};

    vector<Point> connected_point_list;

    int score = board.cell[p.x][p.y];
    queue<Point> search;
    search.push(p);
    is_used[p.x][p.y] = true;
    connected_point_list.push_back(p);
    while (!search.empty())
    {
        Point cp = search.front();
        search.pop();

        Point dp[4] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
        for (const auto &d : dp)
        {
            Point np = {cp.x + d.x, cp.y + d.y};
            if (insert_search(np, board.cell[np.x][np.y], score, is_used, search))
            {
                connected_point_list.push_back(np);
            }
        }
    }

    return connected_point_list;
}

vector<Point> connected_point_with_cultivation(const Board &board, const Board &cultivation, bool (&is_used)[50][50], const Point &p)
{
    if (is_used[p.x][p.y])
        return {};

    vector<Point> connected_point_list;

    int score = board.cell[p.x][p.y] + cultivation.cell[p.x][p.y];
    queue<Point> search;
    search.push(p);
    is_used[p.x][p.y] = true;
    connected_point_list.push_back(p);
    while (!search.empty())
    {
        Point cp = search.front();
        search.pop();

        Point dp[4] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
        for (const auto &d : dp)
        {
            Point np = {cp.x + d.x, cp.y + d.y};
            if (insert_search(np, board.cell[np.x][np.y] + cultivation.cell[np.x][np.y], score, is_used, search))
            {
                connected_point_list.push_back(np);
            }
        }
    }

    return connected_point_list;
}

int first_evaluate(const Board &board, TakenGroup &taken_group)
{
    bool is_used[50][50] = {};
    multiset<int> harvest_list;
    FOR_LT(i, 0, 50)
    {
        FOR_LT(j, 0, 50)
        {
            auto connected_point_list = conected_point(board, is_used, {i, j});
            if (connected_point_list.size() < board.cell[i][j])
                continue;

            int score = connected_point_list.size() * board.cell[i][j];
            for (const auto &point : connected_point_list)
            {
                taken_group.belonging[point] = taken_group.next_belonging_idx;
                taken_group.belonging_point_list[taken_group.next_belonging_idx].push_back(point);
            }
            taken_group.belonging_to_score[taken_group.next_belonging_idx] = score;
            taken_group.score.insert(score);

            taken_group.next_belonging_idx++;
        }
    }

    int total_score = 0;
    for (int score : taken_group.score)
    {
        total_score += score;
    }

    return total_score;
}

int evaluate_score_set(const multiset<int> &score_set, int max_take_count)
{
    int take_count = 0;
    int total_score = 0;
    for (auto it = score_set.rbegin(); it != score_set.rend(); it++)
    {
        if (max_take_count <= take_count)
            break;
        total_score += *it;
        take_count++;
    }

    return total_score;
}

bool is_belonging(const Point &p, const TakenGroup &taken_group)
{
    if (taken_group.belonging.count(p) == 0)
        return false;
    int belonging = taken_group.belonging.at(p);
    if (taken_group.belonging_to_score.count(belonging) == 0)
        return false;
    return true;
}

void insert_delete_belonging_list(const Point &p, const TakenGroup &taken_group, set<int> &delete_belonging_list)
{
    if (!is_belonging(p, taken_group))
        return;
    int belonging = taken_group.belonging.at(p);
    delete_belonging_list.insert(belonging);
}

int evaluate_in_diff(const Board &board, Board &cultivation, const vector<pair<Point, int>> &diff_list, TakenGroup &taken_group)
{
    vector<Point> check_point_list;

    set<int> delete_belonging_list;
    int cultivation_count_diff = 0;
    for (const auto &diff : diff_list)
    {
        check_point_list.push_back(diff.first);
        cultivation.cell[diff.first.x][diff.first.y] += diff.second;
        cultivation_count_diff += diff.second;
        insert_delete_belonging_list(diff.first, taken_group, delete_belonging_list);
    }

    for (int delete_belonging : delete_belonging_list)
    {
        for (const auto &point : taken_group.belonging_point_list.at(delete_belonging))
        {
            check_point_list.push_back(point);
        }
    }

    bool is_used[50][50] = {};
    vector<int> add_score_list;
    for (const auto &point : check_point_list)
    {
        insert_delete_belonging_list(point, taken_group, delete_belonging_list);

        auto connected_point_list = connected_point_with_cultivation(board, cultivation, is_used, point);
        int cell_score = board.cell[point.x][point.y] + cultivation.cell[point.x][point.y];
        if (connected_point_list.size() < cell_score)
            continue;
        for (const auto &connected_point : connected_point_list)
        {
            insert_delete_belonging_list(connected_point, taken_group, delete_belonging_list);
        }

        add_score_list.push_back(cell_score * connected_point_list.size());
    }

    for (int delete_belonging : delete_belonging_list)
    {
        int score = taken_group.belonging_to_score.at(delete_belonging);
        taken_group.score.erase(taken_group.score.find(score));
    }
    for (int add_score : add_score_list)
    {
        taken_group.score.insert(add_score);
    }

    int total_score = evaluate_score_set(taken_group.score, 2500 - (taken_group.cultivation_count + cultivation_count_diff));

    for (int add_score : add_score_list)
    {
        taken_group.score.erase(taken_group.score.find(add_score));
    }
    for (int delete_belonging : delete_belonging_list)
    {
        int score = taken_group.belonging_to_score.at(delete_belonging);
        taken_group.score.insert(score);
    }
    for (const auto &diff : diff_list)
    {
        cultivation.cell[diff.first.x][diff.first.y] -= diff.second;
    }

    return total_score;
}

void update(const Board &board, Board &cultivation, const vector<pair<Point, int>> &diff_list, TakenGroup &taken_group)
{
    vector<Point> check_point_list;

    set<int> delete_belonging_list;
    int cultivation_count_diff = 0;
    for (const auto &diff : diff_list)
    {
        check_point_list.push_back(diff.first);
        cultivation.cell[diff.first.x][diff.first.y] += diff.second;
        cultivation_count_diff += diff.second;

        insert_delete_belonging_list(diff.first, taken_group, delete_belonging_list);
    }
    taken_group.cultivation_count += cultivation_count_diff;

    for (int delete_belonging : delete_belonging_list)
    {
        for (const auto &point : taken_group.belonging_point_list.at(delete_belonging))
        {
            check_point_list.push_back(point);
        }
    }

    bool is_used[50][50] = {};
    for (const auto point : check_point_list)
    {
        auto connected_point_list = connected_point_with_cultivation(board, cultivation, is_used, point);
        int cell_score = board.cell[point.x][point.y] + cultivation.cell[point.x][point.y];
        if (connected_point_list.size() < cell_score)
            continue;

        int score = connected_point_list.size() * cell_score;
        for (const auto &connected_point : connected_point_list)
        {
            insert_delete_belonging_list(connected_point, taken_group, delete_belonging_list);
            taken_group.belonging[connected_point] = taken_group.next_belonging_idx;
            taken_group.belonging_point_list[taken_group.next_belonging_idx].push_back(connected_point);
        }
        taken_group.belonging_to_score[taken_group.next_belonging_idx] = score;
        taken_group.score.insert(score);
        taken_group.next_belonging_idx++;
    }

    for (int delete_belonging : delete_belonging_list)
    {
        int score = taken_group.belonging_to_score[delete_belonging];
        taken_group.belonging_to_score.erase(delete_belonging);
        taken_group.belonging_point_list.erase(delete_belonging);
        taken_group.score.erase(taken_group.score.find(score));
    }
}

template <typename T>
bool has_elem(const vector<T> &vec, const T &elem)
{
    for (const auto &v : vec)
    {
        if (v == elem)
            return true;
    }
    return false;
}

void add_cell(const Point &p, vector<Point> &point_list, vector<Point> &candidate_point_list)
{
    point_list.push_back(p);
    Point dd[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (const auto &d : dd)
    {
        Point np = {p.x + d.x, p.y + d.y};
        if (!Board::is_in_board(np))
            continue;
        if (has_elem(point_list, np))
            continue;
        if (has_elem(candidate_point_list, np))
            continue;

        candidate_point_list.push_back(np);
    }
}

vector<pair<Point, int>> random_operation(const Board &board, const Board &cultivation, const BoardDencity &board_dencity, mt19937 &rand)
{
    // uniform_int_distribution<int> area_dist(0, board_dencity.total_dencity - 1);
    // auto area_head = board_dencity.sum_dencity_to_pos(area_dist(rand));
    // std::cerr << area_idx.first << " " << area_idx.second << endl;

    uniform_int_distribution<int> pos_dist(0, 49);
    int x = pos_dist(rand);
    int y = pos_dist(rand);
    // uniform_int_distribution<int> pos_dist(0, BoardDencity::kAreaSize - 1);
    // int x = pos_dist(rand) + area_head.x;
    // int y = pos_dist(rand) + area_head.y;
    //  int max_count = ceil(board.average_filter({ x, y }));
    // cerr << max_count << " " << board.cell[x][y] << endl;

    int kMinCount = 1;
    uniform_int_distribution<int> count_dist(kMinCount, board.cell[x][y]);
    int point_count = count_dist(rand);
    vector<Point> point_list;
    {
        vector<Point> candidate_point_list;
        add_cell({x, y}, point_list, candidate_point_list);

        while (point_list.size() < point_count)
        {
            uniform_int_distribution<int> sel_dist(0, candidate_point_list.size() - 1);
            int sel = sel_dist(rand);
            Point np = candidate_point_list[sel];

            candidate_point_list.erase(candidate_point_list.begin() + sel);
            add_cell(np, point_list, candidate_point_list);
        }
    }
    int max_score = 0;
    for (const auto &p : point_list)
    {
        max_score = max(max_score, board.cell[p.x][p.y]);
    }

    vector<pair<Point, int>> diff_list;
    for (const auto &p : point_list)
    {
        int diff = max_score - (board.cell[p.x][p.y] + cultivation.cell[p.x][p.y]);
        diff_list.push_back({p, diff});
    }
    return diff_list;
}

bool is_random_chosen(double temprature, mt19937 &rand)
{
    std::uniform_real_distribution<> dist(0.0, 1.0);
    double rand_value = dist(rand);
    return rand_value < temprature;
}

void annul_meaningless_cultivation(Board &cultivation, TakenGroup &taken_group)
{
    FOR_LT(i, 0, 50)
    {
        FOR_LT(j, 0, 50)
        {
            if (taken_group.belonging.count({i, j}) == 0)
            {
                taken_group.cultivation_count -= cultivation.cell[i][j];
                cultivation.cell[i][j] = 0;
                continue;
            }
            int belonging = taken_group.belonging.at({i, j});
            if (taken_group.belonging_to_score.count(belonging) == 0)
            {
                taken_group.cultivation_count -= cultivation.cell[i][j];
                cultivation.cell[i][j] = 0;
                continue;
            }
        }
    }
}

bool can_remove(const Board &board, Board &cultivation, const Point &p, TakenGroup &taken_group)
{
    int current_value = board.cell[p.x][p.y] + cultivation.cell[p.x][p.y];
    int cultivation_value = cultivation.cell[p.x][p.y];

    bool is_used[50][50] = {};
    int prev_connected_count = 0;
    for (const auto &cp : taken_group.belonging_point_list[taken_group.belonging[p]])
    {
        if (board.cell[cp.x][cp.y] + cultivation.cell[cp.x][cp.y])
            prev_connected_count++;
    }

    cultivation.cell[p.x][p.y] = 0;

    int connected_count = 0;
    for (const auto &cp : taken_group.belonging_point_list[taken_group.belonging[p]])
    {
        if (board.cell[cp.x][cp.y] + cultivation.cell[cp.x][cp.y] != current_value)
            continue;
        auto connected_points = connected_point_with_cultivation(board, cultivation, is_used, cp);
        if (connected_points.size() < current_value)
            continue;
        connected_count += connected_points.size();
    }
    assert(connected_count != prev_connected_count);

    cultivation.cell[p.x][p.y] = cultivation_value;

    return (connected_count + 1 == prev_connected_count);
}

void easen_cultivation(const Board &board, Board &cultivation, TakenGroup &taken_group)
{
    array<vector<Point>, 9> value_point_list;
    FOR_LT(i, 0, 50)
    {
        FOR_LT(j, 0, 50)
        {
            int value = board.cell[i][j] + cultivation.cell[i][j];
            value_point_list[value - 1].push_back({i, j});
        }
    }

    int reduce_count = 0;
    FOR_LT(i, 0, 9)
    {
        int current_value = i + 1;
        FOR_LT(pi, 0, value_point_list[i].size())
        {
            const auto &p = value_point_list[i][pi];
            if (current_value != board.cell[p.x][p.y] + cultivation.cell[p.x][p.y])
                continue;
            if (!is_belonging(p, taken_group))
                continue;

            constexpr Point dd[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            for (const auto &d : dd)
            {
                Point np = {p.x + d.x, p.y + d.y};
                if (!Board::is_in_board(np))
                    continue;

                int nvalue = board.cell[np.x][np.y] + cultivation.cell[np.x][np.y];

                if (nvalue <= current_value || current_value < board.cell[np.x][np.y])
                    continue;
                if (!is_belonging(np, taken_group))
                    continue;

                if (!can_remove(board, cultivation, np, taken_group))
                    continue;

                cultivation.cell[np.x][np.y] = current_value - board.cell[np.x][np.y];
                reduce_count += nvalue - current_value;
                value_point_list[i].push_back(np);
            }
        }
    }
    // cerr << reduce_count << endl;

    Board unified_board;
    FOR_LT(i, 0, 50)
    {
        FOR_LT(j, 0, 50)
        {
            unified_board.cell[i][j] = board.cell[i][j] + cultivation.cell[i][j];
        }
    }

    TakenGroup new_taken_group;
    int total_value = first_evaluate(unified_board, new_taken_group);
    int prev_cultivation_count = taken_group.cultivation_count;
    taken_group = std::move(new_taken_group);
    taken_group.cultivation_count = prev_cultivation_count - reduce_count;

    // cerr << reduce_count << endl;
}

void final_greedy(const Board &board, Board &cultivation, TakenGroup &taken_group)
{
    auto score_it = taken_group.score.rbegin();

    int count_rem = 2500 - taken_group.cultivation_count;
    REP(2500 - taken_group.cultivation_count)
    {
        score_it++;
        count_rem--;
        if (score_it == taken_group.score.rend())
        {
            break;
        }
    }

    multimap<double, pair<int, Point>> candidate_map;
    FOR_LT(i, 0, 50)
    {
        FOR_LT(j, 0, 50)
        {
            if (is_belonging({i, j}, taken_group))
                continue;

            constexpr Point dd[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            bool found_candidate = false;
            int diff = INT_MAX;
            for (const auto &d : dd)
            {
                Point np = {i + d.x, j + d.y};
                if (!Board::is_in_board(np))
                    continue;
                if (!is_belonging(np, taken_group))
                    continue;
                int ns = board.cell[np.x][np.y] + cultivation.cell[np.x][np.y];
                if (board.cell[i][j] < ns)
                {
                    found_candidate = true;
                    int current_diff = ns - board.cell[i][j];
                    if (current_diff < diff)
                    {
                        diff = current_diff;
                    }
                }
            }
            if (found_candidate)
            {
                double ratio = ((double)board.cell[i][j] + diff) / diff;
                candidate_map.insert({ratio, {diff, Point{i, j}}});
            }
        }
    }

    int add_count = 0;
    if (count_rem != 0)
    {
        auto cand_it = candidate_map.rbegin();
        while (count_rem != 0 && cand_it != candidate_map.rend())
        {
            const Point &p = cand_it->second.second;
            cultivation.cell[p.x][p.y] += cand_it->second.first;
            add_count += cand_it->second.first;
            count_rem--;
            cand_it++;
        }
    }
    else
    {
        int score_diff = 0;
        auto cand_it = candidate_map.rbegin();
        while (cand_it != candidate_map.rend())
        {
            const Point &p = cand_it->second.second;
            int diff = cand_it->second.first;
            int add_score = board.cell[p.x][p.y] + diff;

            auto score_it_tmp = score_it;
            int org_score = 0;
            bool is_ok = true;
            REP(diff)
            {
                org_score += *score_it_tmp;
                if (score_it_tmp == taken_group.score.rbegin())
                {
                    is_ok = false;
                    break;
                }
                score_it_tmp--;
            }
            if (!is_ok)
                break;

            if (org_score < add_score)
            {
                score_diff += add_score - org_score;
                cultivation.cell[p.x][p.y] += diff;
                add_count += cand_it->second.first;
                if (score_it == taken_group.score.rbegin())
                    break;
                score_it--;
            }
            cand_it++;
        }
        // cerr << score_diff << endl;
    }

    Board unified_board;
    FOR_LT(i, 0, 50)
    {
        FOR_LT(j, 0, 50)
        {
            unified_board.cell[i][j] = board.cell[i][j] + cultivation.cell[i][j];
        }
    }

    TakenGroup new_taken_group;
    int total_value = first_evaluate(unified_board, new_taken_group);
    int prev_cultivation_count = taken_group.cultivation_count;
    taken_group = std::move(new_taken_group);
    taken_group.cultivation_count = prev_cultivation_count + add_count;
}

void final_choice(const Board &board, Board &cultivation, const TakenGroup &taken_group)
{
    struct EffectivenessInfo
    {
        int belonging;
        int count;
    };

    multimap<double, EffectivenessInfo> effectiveness_map;
    for (const auto &belonging : taken_group.belonging_point_list)
    {
        int cc = 1;
        int belonging_id = belonging.first;
        for (const auto &p : taken_group.belonging_point_list.at(belonging_id))
        {
            cc += cultivation.cell[p.x][p.y];
        }
        double ratio = (double)taken_group.belonging_to_score.at(belonging_id) / cc;
        effectiveness_map.insert({ratio, {belonging_id, cc}});
    }

    int remain_count = 2500;
    vector<int> discard_list;
    for (auto it = effectiveness_map.rbegin(); it != effectiveness_map.rend(); it++)
    {
        if (remain_count < it->second.count)
        {
            // std::cerr << "disbel : " << it->second.belonging << " " << it->second.count << " " << taken_group.belonging_to_score.at(it->second.belonging) << endl;
            discard_list.push_back(it->second.belonging);
            continue;
        }
        remain_count -= it->second.count;
    }

    // cerr << "dis : " << discard_list.size() << " " << effectiveness_map.size() << endl;
    for (int discord_beloning : discard_list)
    {
        for (const auto &p : taken_group.belonging_point_list.at(discord_beloning))
        {
            cultivation.cell[p.x][p.y] = 0;
        }
    }
}

Board annealing(const Board &board)
{
    auto start_time = chrono::high_resolution_clock::now();

    TakenGroup taken_group = {};
    int total_score = first_evaluate(board, taken_group);
    BoardDencity board_dencity(board);

    double temperature = 0.0;
    mt19937 rand = mt19937(time(nullptr));

    Board cultivation = {};
    static int count = 0;
    while (true)
    {
        {
            auto now = chrono::high_resolution_clock::now();
            auto time_diff = now - start_time;
            if (chrono::milliseconds(1950) <= time_diff)
            {
                break;
            }
        }
        count++;

        auto diff_list = random_operation(board, cultivation, board_dencity, rand);

        int next_total_score = evaluate_in_diff(board, cultivation, diff_list, taken_group);
        // cerr << total_score << " " << next_total_score << " " << taken_group.cultivation_count << endl;
        if (next_total_score == 0)
            continue;

        if (total_score < next_total_score || is_random_chosen(temperature, rand))
        {
            update(board, cultivation, diff_list, taken_group);
            total_score = next_total_score;
        }
    }
    // cerr << count << endl;

    annul_meaningless_cultivation(cultivation, taken_group);
    easen_cultivation(board, cultivation, taken_group);
    final_greedy(board, cultivation, taken_group);
    final_choice(board, cultivation, taken_group);

    return cultivation;
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    cout << fixed << setprecision(20);

    int n, m;
    cin >> n >> m;

    Board board;
    FOR_LT(i, 0, 50)
    {
        FOR_LT(j, 0, 50)
        {
            cin >> board.cell[i][j];
        }
    }

    Board cultivation = annealing(board);

    int cc = 0;
    FOR_LT(i, 0, 50)
    {
        FOR_LT(j, 0, 50)
        {
            cc += cultivation.cell[i][j];
        }
    }

    FOR_LT(i, 0, 50)
    {
        FOR_LT(j, 0, 50)
        {
            board.cell[i][j] += cultivation.cell[i][j];
        }
    }

    auto harvest_points = execute_harvest(board, 2500 - cc);
    FOR_LT(i, 0, 50)
    {
        FOR_LT(j, 0, 50)
        {
            REP(cultivation.cell[i][j])
            {
                cout << 1 << " " << i << " " << j << endl;
            }
        }
    }

    for (const auto &point : harvest_points)
    {
        cout << 2 << " " << point.x << " " << point.y << endl;
    }

    return 0;
}
