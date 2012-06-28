#!/usr/bin/env python

import sys
from apiclient.discovery import build
from pprint import pprint as pp
import codecs

def main(args):
    query = args[1]
    output = args[2]

    apikey_file = open("apikey", "r")
    key = apikey_file.read().strip()
    apikey_file.close()

    # Build search service
    service = build("customsearch", "v1", developerKey=key)

    # Get search results from en.wikipedia.org
    res = service.cse().list(q=query, cx="*.org", siteSearch="http://en.wikipedia.org").execute()

    out = codecs.open(output, "w", "utf-8")
    out.write(query + "\n")

    for r in res['items']:
        out.write(r["formattedUrl"] + "\n")
        out.write(r["snippet"] + "\n")

    out.close()

if __name__ == "__main__":
    main(sys.argv)
