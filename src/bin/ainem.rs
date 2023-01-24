use std::collections::BinaryHeap;

use proconio::input;

const DR: [usize; 4] = [1, 0, !0, 0];
const DC: [usize; 4] = [0, 1, 0, !0];

// Partial orderなものをTotal orderにする
// https://qiita.com/hatoo@github/items/fa14ad36a1b568d14f3e
#[derive(PartialEq, PartialOrd)]
pub struct Total<T>(pub T);

impl<T: PartialEq> Eq for Total<T> {}

impl<T: PartialOrd> Ord for Total<T> {
    fn cmp(&self, other: &Total<T>) -> std::cmp::Ordering {
        self.0.partial_cmp(&other.0).unwrap()
    }
}
fn main() {
    input! {
        n:usize,
            mut m:usize,
        aa:[[usize;n];n]
    }
    let mut a = aa.clone();
    let mut seen = vec![vec![false; n]; n];
    let mut candidates = BinaryHeap::new();
    let points = matrix_to_points(&a);
    for tt in 0..3 {
        for threshold in 1..=[3, 6, 9][tt] {
            for &(i, j) in &points {
                if let Some((sc, list)) = count(&a, &seen, i, j, threshold, false) {
                    candidates.push((sc, i, j, list));
                }
            }
        }
        eprintln!("len {}", candidates.len());
        while let Some((Total(_sc), i, j, list)) = candidates.pop() {
            let dup = list
                .iter()
                .filter(|&&(c, r)| seen[c][r])
                .collect::<Vec<_>>();
            if dup.len() > 0 {
                continue;
            }
            for (c, r) in list {
                seen[c][r] = true;
                a[c][r] = a[i][j];
            }
        }
    }

    // 収穫パート
    let mut seen = vec![vec![false; 50]; 50];
    let mut output = vec![];
    for i in 0..50 {
        for j in 0..50 {
            if seen[i][j] {
                continue;
            }
            if let Some((_, list)) = count(&a, &seen, i, j, 1, false) {
                for &(c, r) in &list {
                    seen[c][r] = true;
                }
                let out = format!("2 {} {}", i, j);
                output.push((a[i][j] * list.len(), list.len(), (i, j), out));
            }
        }
    }
    output.sort();
    for i in 0..50 {
        for j in 0..50 {
            if !seen[i][j] {
                a[i][j] = aa[i][j]
            }
        }
    }
    let output_incre = make_output(&aa, &a);
    let mut rest_m = m.saturating_sub(output.len() + output_incre.len());
    if rest_m == 0 {
        println!("{}", output_incre.join("\n"));
        m -= output_incre.len();
        while let Some((_, _cost, _, out)) = output.pop() {
            if m < 1 {
                break;
            }
            m -= 1;
            println!("{}", out);
        }
        eprintln!("tarinai");
    } else {
        // 余った回数でなんかしたい
        // seenがfalseなところをたどってseenがtrue(収穫済)のところと同じ値にする

        let mut cand = vec![];
        for &(_, _, (i, j), _) in &output {
            if a[i][j] < 6 {
                continue;
            }
            seen[i][j] = false;
            for threshold in 3..10 {
                if let Some((_, list)) = count(&a, &seen, i, j, threshold, true) {
                    let mut cost = 0;
                    for &(c, r) in &list {
                        cost += a[i][j] - a[c][r];
                    }
                    if cost == 0 {
                        continue;
                    }
                    let sc = Total(a[i][j] as f64 * list.len() as f64 / cost as f64);
                    cand.push((sc, a[i][j], cost, list));
                }
            }
            seen[i][j] = true;
        }
        cand.sort();
        eprintln!("rest_m {} cand {}", rest_m, cand.len());
        while let Some((_, aij, cost, list)) = cand.pop() {
            if cost - 1 > rest_m {
                continue;
            }
            let mut real_cost = 0;
            for (c, r) in list {
                if seen[c][r] {
                    continue;
                }
                real_cost += aij - a[c][r];
                a[c][r] = aij;
            }
            eprintln!("{} {}", rest_m, real_cost);
            if rest_m < real_cost {
                break;
            }
            rest_m -= real_cost;
        }
        let output_incre = make_output(&aa, &a);
        println!("{}", output_incre.join("\n"));
        let mut rest = 2500 - output_incre.len();
        while let Some((_, _cost, _, out)) = output.pop() {
            if rest == 1 {
                break;
            }
            println!("{}", out);
            rest -= 1;
        }
        eprintln!("tariruhazu");
    }
}

fn make_output(a: &Vec<Vec<usize>>, b: &Vec<Vec<usize>>) -> Vec<String> {
    let mut out = vec![];
    for i in 0..50 {
        for j in 0..50 {
            for _ in 0..b[i][j] - a[i][j] {
                out.push(format!("1 {} {}", i, j))
            }
        }
    }
    out
}

fn matrix_to_points(a: &Vec<Vec<usize>>) -> Vec<(usize, usize)> {
    let mut ret = vec![];
    let mut ps = vec![];
    for i in 0..50 {
        for j in 0..50 {
            ps.push((a[i][j], (i, j)));
        }
    }
    ps.sort();
    while let Some((_, p)) = ps.pop() {
        ret.push(p);
    }
    ret
}
fn count(
    a: &Vec<Vec<usize>>,
    seen: &Vec<Vec<bool>>,
    cr: usize,
    cc: usize,
    threshold: usize,
    flag: bool,
) -> Option<(Total<f64>, Vec<(usize, usize)>)> {
    let mut list = vec![];
    list.push((cr, cc));
    let mut i = 0;
    let mut cost = 1;
    while i < list.len() {
        for j in 0..4 {
            let nr = list[i].0.wrapping_add(DR[j]);
            let nc = list[i].1.wrapping_add(DC[j]);
            let np = (nr, nc);

            if nr < 50
                && nc < 50
                && ((flag && a[nr][nc] == 1) || !seen[nr][nc])
                && a[nr][nc] <= a[cr][cc]
                && a[cr][cc] - a[nr][nc] < threshold
                && !list.contains(&np)
            {
                list.push(np);
                cost += a[cr][cc] - a[nr][nc];
            }
        }
        i += 1;
    }
    if flag || list.len() >= a[cr][cc] {
        let score = list.len() * a[cr][cc];

        let ret = score as f64 / cost as f64;
        return Some((Total(ret), list));
    }
    None
}
