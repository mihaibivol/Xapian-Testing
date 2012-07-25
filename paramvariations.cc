#include <xapian.h>

#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include <cstdlib> // For exit().
#include <cstring>
#include <sstream>

#define BUFF_LEN 1024

using namespace std;

void
test_file(
	const char *		    filename,
	Xapian::QueryParser &	    qp,
	Xapian::Enquire &	    enquire,
	int			    variation_index)
{
    ifstream file(filename);
    char buff[BUFF_LEN];

    // Get the ground truth snippets for each url.
    while (!file.eof()) {
	file.getline(buff, BUFF_LEN);
	string query_s(buff);

	Xapian::Query query = qp.parse_query(query_s);
	enquire.set_query(query);
	// Find the top 15 results for the query.
	Xapian::MSet matches = enquire.get_mset(0, 15);

	Xapian::Snipper snipper;
	Xapian::Stem stemmer("english");
	snipper.set_stemmer(stemmer);
	snipper.set_mset(matches);

	int dn[6] =	{10,  5,  5,  5, 10, 10};
	int ws[6] =	{10, 30, 10, 20, 40, 20};
	double sc[6] =  {.5, .5, .9, .5, .5, .9};

	snipper.set_rm_docno(dn[variation_index]);
	snipper.set_mset(matches);
	snipper.set_smoothing_coef(sc[variation_index]);
	snipper.set_window_size(ws[variation_index]);

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

	    string snippet = snipper.generate_snippet(gen_text);
	    cout << "Query: \"" << query_s << "\" Document: \""
		 << url << " Snippet: \"" << snippet << "\"\n";
	}
    }
    file.close();
}

int
main(int argc, char **argv)
{
    try {
        // We require at least 4 command line arguments.
        if (argc != 4) {
	    int rc = 1;
	    if (argv[1]) {
		if (strcmp(argv[1], "--version") == 0) {
		    cout << "evalgen" << endl;
		    exit(0);
		}
		if (strcmp(argv[1], "--help") == 0) {
		    rc = 0;
		}
	    }
	    cout << "Usage: " << argv[0] << " PATH_TO_DATABASE QUERYFILE PARAM_NO" << endl;
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
	test_file(argv[2], qp, enquire, atoi(argv[3]));
    } catch (const Xapian::Error &e) {
        cout << e.get_description() << endl;
        exit(1);
    }
}
