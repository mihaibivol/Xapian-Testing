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
    // Find the top 10 results for the query.

    // Display the results.
    cout << matches.get_matches_estimated() << " results found.\n";
    cout << "Matches 1-" << matches.size() << ":\n" << endl;

    for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); ++i) {
	// Get only the sample from the document data.
	string gen_text = i.get_document().get_data();
	// Get url name
	string url = gen_text.substr(5, gen_text.find('\n') - 5);
	cout << "for URL: " << url << endl;
	string sample_mark("sample=");
	string type_mark("type=");

	size_t sample_pos = gen_text.find(sample_mark) + sample_mark.size();
	gen_text.erase(gen_text.begin(), gen_text.begin() + sample_pos);

	size_t type_pos = gen_text.rfind(type_mark);
	gen_text.erase(gen_text.begin() + type_pos, gen_text.end());

	cout << snipper.generate_snippet(matches, gen_text) << endl;
	cout << ground_truth[url] << endl;
	cout << endl;

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
