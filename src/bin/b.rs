use proconio::input;

fn main() {
    input! {
        n:usize,
        mut m:usize,
        mut a:[[usize;n];n]
    }
    let mut output = vec![];
    for i in 0..n {
        while a[i][0] < 9 {
            a[i][0] += 1;
            m -= 1;
            output.push(format!("1 {} 0", i))
        }
        while a[0][i] < 9 {
            a[0][i] += 1;
            m -= 1;
            output.push(format!("1 0 {}", i))
        }
    }
    for i in 0..n {
        for j in 0..n {
            if 9 - a[i][j] < m {
                while a[i][j] < 9 {
                    a[i][j] += 1;
                    m -= 1;
                    output.push(format!("1 {} {}", i, j))
                }
            }
        }
    }

    output.push(format!("2 0 0"));
    println!("{}", output.join("\n"))
}
