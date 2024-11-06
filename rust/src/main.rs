use zim::Zim;
use std::hash::Hash;
use std::process::exit;
use std::collections::HashSet;


fn main() {
    println!("Hello, world!");

    let zim = match Zim::new("/home/jonas/.local/share/kiwix/wikipedia_en_100_nopic_2024-06.zim"){
        Ok(z) => z,
        Err(e) => {println!("{e:?}"); exit(1)},
    };
    println!("{}", zim.article_count());

    let mut urls: HashSet<String> = HashSet::new();
    let mut titles: HashSet<String> = HashSet::new();

    for i in 0..zim.article_count() as u32{
        let dir_entry = match zim.get_by_url_index(i){
            Ok(d) => d,
            Err(e) => {println!("{e:?}"); continue},
        };

        println!("title: {}, url: {}", dir_entry.title, dir_entry.url);
        urls.insert(dir_entry.url);
        titles.insert(dir_entry.title);
    }

    println!("{}", urls.len());
    println!("{}", titles.len());
}
