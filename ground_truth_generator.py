#!/usr/bin/env python

import sys
from apiclient.discovery import build
from pprint import pprint as pp

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

    out = open(output, "w")

    for r in res['items']:
        out.write(r["formattedUrl"] + "\n")
        out.write(query + "\n")
        out.write(r["snippet"] + "\n")

if __name__ == "__main__":
    main(sys.argv)
