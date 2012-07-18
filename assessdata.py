#!/usr/bin/env python
import sys

class ParamResult(object):
    def __init__(self):
        self.number = {
                1 : 0,
                2 : 0,
                3 : 0,
                4 : 0,
                5 : 0}

    def get_avg_score(self):
        total_score = .0
        total_number = 0
        for val in self.number:
            total_score += val * self.number[val]
            total_number += self.number[val]

        return total_score / total_number

    def print_data(self):
        print "\t\tAverage: %f" % self.get_avg_score()
        for score in self.number:
            result_str = "\t\t%d results for %d score"
            print result_str % (self.number[score], score)


class SnippetSetResult(object):
    def __init__(self):
        self.result = [ParamResult(), ParamResult(), ParamResult(),
                       ParamResult(), ParamResult(), ParamResult()]

    def add_result(self, cnt_no, value):
        self.result[cnt_no].number[value] += 1

    def print_data(self):
        for cnt in xrange(0, 6):
            print "\tfor %d set" % cnt
            self.result[cnt].print_data()

class Assessor(object):
    def __init__(self, files):
        self.query_results = {}
        self.total_results = SnippetSetResult()
        for f in files:
            self.assess_file(f)

    def assess_file(self, filename):
        f = open(filename)
        eof = False
        while not eof:
            for cnt in xrange(0, 6):
                query = f.readline()
                if not query:
                    eof = True
                    break

                f.readline()
                f.readline()
                score = int(f.readline())

                try:
                    self.query_results[query].add_result(cnt, score)
                except KeyError:
                    self.query_results[query] = SnippetSetResult()
                    self.query_results[query].add_result(cnt, score)

                self.total_results.add_result(cnt, score)

    def print_results(self):
        for query in self.query_results:
            snippet_set_result = self.query_results[query]
            print "Query %s" % query
            self.query_results[query].print_data()

        print
        print "Grand total"
        self.total_results.print_data()

def main(args):
    assessor = Assessor(args[1:])
    assessor.print_results()

if __name__ == "__main__":
            main(sys.argv)
