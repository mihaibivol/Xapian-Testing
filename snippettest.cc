/** @file simplesearch.cc
 * @brief Simple command-line search utility.
 *
 * See "quest" for a more sophisticated example.
 */
/* Copyright (C) 2007,2010 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <xapian.h>

#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include <cstdlib> // For exit().
#include <cstring>

#define BUFF_LEN 1024

using namespace std;

string
html_escape(const string &str)
{
    string res;
    string::size_type p = 0;
    while (p < str.size()) {
	char ch = str[p++];
	switch (ch) {
	    case '<':
	        res += "&lt;";
	        continue;
	    case '>':
	        res += "&gt;";
	        continue;
	    case '&':
	        res += "&amp;";
	        continue;
	    case '"':
	        res += "&quot;";
	        continue;
	    default:
	        res += ch;
	}
    }
    return res;
}

static int word_in_list(const string& word, const string& list)
{
    string::size_type split = 0, split2;
    int count = 0;
    while ((split2 = list.find(' ', split)) != string::npos) {
	if (word.size() == split2 - split) {
	    if (memcmp(word.data(), list.data() + split, word.size()) == 0)
		return count;
	}
	split = split2 + 1;
	++count;
    }
    if (word.size() == list.size() - split) {
	if (memcmp(word.data(), list.data() + split, word.size()) == 0)
	    return count;
    }
    return -1;
}

static string
html_highlight(const string &s, const string &list,
	       const string &bra, const string &ket)
{
    Xapian::Stem stemmer("english");

    string res;

    using namespace Xapian::Unicode;

    Xapian::Utf8Iterator j(s);
    const Xapian::Utf8Iterator s_end;
    while (true) {
	Xapian::Utf8Iterator first = j;
	while (first != s_end && !is_wordchar(*first)) ++first;
	if (first == s_end) break;
	Xapian::Utf8Iterator term_end;
	string term;
	string word;
	const char *l = j.raw();
	if (*first < 128 && isupper(*first)) {
	    j = first;
	    Xapian::Unicode::append_utf8(term, *j);
	    while (++j != s_end && *j == '.' && ++j != s_end && *j < 128 && isupper(*j)) {
		Xapian::Unicode::append_utf8(term, *j);
	    }
	    if (term.length() < 2 || (j != s_end && is_wordchar(*j))) {
		term.resize(0);
	    }
	    term_end = j;
	}
	if (term.empty()) {
	    j = first;
	    while (is_wordchar(*j)) {
		Xapian::Unicode::append_utf8(term, *j);
		++j;
		if (j == s_end) break;
		if (*j == '&' || *j == '\'') {
		    Xapian::Utf8Iterator next = j;
		    ++next;
		    if (next == s_end || !is_wordchar(*next)) break;
		    term += *j;
		    j = next;
		}
	    }
	    term_end = j;
	    if (j != s_end && (*j == '+' || *j == '-' || *j == '#')) {
		string::size_type len = term.length();
		if (*j == '#') {
		    term += '#';
		    do { ++j; } while (j != s_end && *j == '#');
		} else {
		    while (j != s_end && (*j == '+' || *j == '-')) {
			Xapian::Unicode::append_utf8(term, *j);
			++j;
		    }
		}
		if (term.size() - len > 3 || (j != s_end && is_wordchar(*j))) {
		    term.resize(len);
		} else {
		    term_end = j;
		}
	    }
	}
	j = term_end;
	term = Xapian::Unicode::tolower(term);
	int match = word_in_list(term, list);
	if (match == -1) {
	    string stem = "Z";
	    stem += stemmer(term);
	    match = word_in_list(stem, list);
	}
	if (match >= 0) {
	    res += html_escape(string(l, first.raw() - l));
	    if (!bra.empty()) {
		res += bra;
	    } else {
		static const char * colours[] = {
		    "ffff66", "99ff99", "99ffff", "ff66ff", "ff9999",
		    "990000", "009900", "996600", "006699", "990099"
		};
		size_t idx = match % (sizeof(colours) / sizeof(colours[0]));
		const char * bg = colours[idx];
		if (strchr(bg, 'f')) {
		    res += "<b style=\"color:black;background-color:#";
		} else {
		    res += "<b style=\"color:white;background-color:#";
		}
		res += bg;
		res += "\">";
	    }
	    word = string(first.raw(), j.raw() - first.raw());
	    res += html_escape(word);
	    if (!bra.empty()) {
		res += ket;
	    } else {
		res += "</b>";
	    }
	} else {
	    res += html_escape(string(l, j.raw() - l));
	}
    }
    if (j != s_end) res += html_escape(string(j.raw(), j.left()));
    return res;
}

void
test_file(
	const char *		    filename,
	Xapian::QueryParser &	    qp,
	Xapian::Enquire &	    enquire)
{
    ifstream file(filename);
    char buff[BUFF_LEN];
    file.getline(buff, BUFF_LEN);

    string query_s(buff);
    // Get the ground truth snippets for each url.
    map<string, string> ground_truth;
    while (!file.eof()) {
	file.getline(buff, BUFF_LEN);
	string url(buff);
	// Hardcoded only for wikipedia files.
	url.replace(0, strlen("en.wikipedia.org/wiki/"), "");
	file.getline(buff, BUFF_LEN);
	string snippet(buff);
	ground_truth[url] = snippet;
    }

    Xapian::Query query = qp.parse_query(query_s);
    enquire.set_query(query);
    Xapian::MSet matches = enquire.get_mset(0, 10);

    Xapian::Snipper snipper;
    Xapian::Stem stemmer("english");
    snipper.set_stemmer(stemmer);
    snipper.set_mset(matches);
    // Find the top 10 results for the query.

    string dump_filename(filename);
    dump_filename += "_0";
    // Display the results.
    for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); ++i) {
	// Get only the sample from the document data.
	string gen_text = i.get_document().get_data();
	// Get url name
	string url = gen_text.substr(5, gen_text.find('\n') - 5);
	string sample_mark("sample=");
	string type_mark("type=");

	size_t sample_pos = gen_text.find(sample_mark) + sample_mark.size();
	gen_text.erase(gen_text.begin(), gen_text.begin() + sample_pos);

	size_t type_pos = gen_text.rfind(type_mark);
	gen_text.erase(gen_text.begin() + type_pos, gen_text.end());


	std::string snippet = snipper.generate_snippet(gen_text);
	snippet = html_highlight(snippet, query_s, "", "");
	if (ground_truth[url].length() != 0) {
	    // Hack, no more than 10 results per file.
	    dump_filename[dump_filename.length() - 1]++;
	    cout << "From: " << url << " <br/> " << endl;
	    cout << "With Query: " << query_s << " <br/> " << endl;
	    cout << snippet << " <br/> " << endl;
	    cout << "Google snippet:" << " <br/> " << endl;
	    cout << ground_truth[url] << " <br/> " << endl;
	    cout << " <br/> " << endl;
	} else {
	    cout << "From: " << url << " <br/> " << endl;
	    cout << "With Query: " << query_s << " <br/> " << endl;
	    cout << snippet << " <br/> " << endl;
	}
	cout << " <br/> " << endl;
    }
    file.close();
}

int
main(int argc, char **argv)
try {
    // We require at least two command line arguments.
    if (argc < 3) {
	int rc = 1;
	if (argv[1]) {
	    if (strcmp(argv[1], "--version") == 0) {
		cout << "snippettest" << endl;
		exit(0);
	    }
	    if (strcmp(argv[1], "--help") == 0) {
		rc = 0;
	    }
	}
	cout << "Usage: " << argv[0] << " PATH_TO_DATABASE FILES" << endl;
	exit(rc);
    }

    // Open the database for searching.
    Xapian::Database db(argv[1]);

    // Start an enquire session.
    Xapian::Enquire enquire(db);

    // Set query parser.
    Xapian::QueryParser qp;
    Xapian::Stem stemmer("english");
    qp.set_stemmer(stemmer);
    qp.set_database(db);
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    for (int i = 2; i < argc; i++) {
	test_file(argv[i], qp, enquire);
    }
} catch (const Xapian::Error &e) {
    cout << e.get_description() << endl;
    exit(1);
}
