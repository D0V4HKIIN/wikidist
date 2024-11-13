use select::document::Document;
use select::predicate::Name;
use std::collections::HashSet;
use std::process::exit;
use std::str::Utf8Error;
use zim::Target::Cluster;
use zim::Zim;
use zim::{DirectoryEntry, MimeType};

/* abandonned because the zim crate is missing get by url/title feature */

// fn is_in_db(page: &str, zim: &Zim) -> bool{
// zim.
// }

fn print_page(entry: &DirectoryEntry, zim: &Zim) -> Result<usize, Utf8Error> {
    let target = entry.target.as_ref().unwrap();

    match target {
        Cluster(x, y) => {
            let cluster = zim.get_cluster(*x).unwrap();
            let blob = cluster.get_blob(*y).unwrap();
            let text = std::str::from_utf8(blob.as_ref())?;
            // println!("{:?}", blob.as_ref());

            // println!("{text}");

            Document::from(text)
                .find(Name("a"))
                .filter_map(|n| n.attr("href"))
                // .filter()
                .for_each(|x| println!("{}", x));

            // match text{
            // Ok(txt) => println!("{}", txt),
            // Err(e) => println!("{e}"),
            // }
        }
        _ => (),
    };
    Ok(1)
}

fn main() {
    println!("Hello, world!");

    // let zim = match Zim::new("/home/jonas/.local/share/kiwix/wikipedia_en_100_nopic_2024-06.zim"){
    let zim = match Zim::new(
        "/home/jonas/.var/app/org.kiwix.desktop/data/kiwix/wikipedia_en_100_nopic_2024-06.zim",
    ) {
        Ok(z) => z,
        Err(e) => {
            println!("{e:?}");
            exit(1)
        }
    };

    println!("{:?}", zim.mime_table);

    let mut urls: HashSet<String> = HashSet::new();
    let mut titles: HashSet<String> = HashSet::new();

    for i in 0..zim.article_count() as u32 {
        let dir_entry = match zim.get_by_url_index(i) {
            Ok(d) => d,
            Err(e) => {
                println!("{e:?}");
                continue;
            }
        };

        match dir_entry.mime_type {
            MimeType::Type(s) => {
                if s != "text/html" {
                    // println!("not txt {}, type {:?}", dir_entry.url, s);
                    continue;
                }
            }
            _ => {
                // println!("not txt {}, type {:?}", dir_entry.url, dir_entry.mime_type);
                continue;
            }
        }

        // let _ = print_page(&dir_entry, &zim);

        // println!("title: {}, url: {}", dir_entry.title, dir_entry.url);
        urls.insert(dir_entry.url);
        titles.insert(dir_entry.title);
    }

    println!("{}", urls.len());
    println!("{:?}", urls);
    println!("{}", titles.len());
    println!("{:?}", titles);
    println!("{}", zim.article_count());
    println!("{}", zim.article_list.len());
    println!("{}", zim.url_list.len());

    let dir: DirectoryEntry = zim
        .get_by_url_index(zim.article_count() as u32 - 1 - 2)
        .unwrap();

    println!("{} at {:#?}", dir.url, dir.target);

    let _ = print_page(&dir, &zim);
}
