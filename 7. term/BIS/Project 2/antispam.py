#!/usr/bin/env python3

import sys
import eml_parser
import json
import datetime
import re
from bs4 import BeautifulSoup

messages = { "err_open": "failed to open file" }
results = {"ok": "OK", "spam": "SPAM", "fail": "FAIL"}
languages = ["en"]
types = ["low", "med", "high"]
scale = {"low": 1, "med": 15, "high": 70}
constatnts = {"no_body": 2, "no_body_small": 1, "no_head": 2, "no_sender": 2, "html": 10, "links": 30, "fonts": 30, "sender_numeric": 10}
weights = {"header": 5, "uppercase": 5}
threshold = 90
percentage = 0.08
message = ""

def strip_html(text):
	soup = BeautifulSoup(text, "html5lib")
	for script in soup(["script", "style"]):
		script.extract() 
	text = soup.get_text()
	return text

def open_email(path):
	with open(path, "rb") as f:
		email = eml_parser.eml_parser.decode_email_b(f.read(), include_raw_body = True)
	return email

def check_sender_recipient(email):
	count = 0
	try:
		if '@' not in email['header']['header']['from'][0] or len(email['header']['header']['from'][0]) == 0:
			count += constatnts['no_sender']
	except:
		count += constatnts['no_sender']

	try:
		if '@' not in email['header']['header']['to'][0] or len(email['header']['header']['to'][0]) == 0:
			count += constatnts['no_sender']
	except:
		count += constatnts['no_sender']

	return count

def is_spam(email, blacklists):
	hit = False
	head = False
	body = False
	size = 65
	penalizations = 0

	global message
	message = ""

	penalizations += check_sender_recipient(email)

	if "header" in email and "subject" in email["header"] and len(email["header"]["subject"]) > 0:
		head = True
	else:
		penalizations += constatnts["no_head"]
		message += "NO_HEAD "

	if "body" in email and len(email["body"]) > 0:
		for part in email["body"]:
			if "content" in part and len(part["content"]) > 0:
				body = True
				size += len(strip_html(part["content"]).split())
			else:
				penalizations += constatnts["no_body_small"]
	else:
		penalizations += constatnts["no_body"]
		message += "NO_BODY "

	for blacklist in blacklists:
		score = 0
		if head:
			score += check_spam_words(email["header"]["subject"], blacklist) * weights["header"]
		if body:
			for part in email["body"]:				
				if "content" in part and len(part["content"]) > 0:
					score += check_spam_words(part["content"], blacklist)
		hit = hit or score + penalizations >= percentage * size or score + penalizations >= threshold
	return hit

def check_spam_words(text, blacklist):
	global message
	score = 0
	base_text = strip_html(text)
	text_lower = base_text.lower()
	urls = re.findall('http[s]?://', text)
	if len(urls) > 6:
		score += constatnts["links"]
		message += "LINKS "
	for t in types:
		for word in blacklist[t]:
			score += text_lower.count(" " + word + " ") * scale[t]
			score += base_text.count(" " + word.upper() + " ") * scale[t] * weights["uppercase"]
			if (t == "high" or t == "med") and text_lower.count(" " + word + " ") > 0:
				message += "[" + word + "]"
	return score

def check_file(path, blacklists):
	global message
	try:
		email = open_email(path)
		if is_spam(email, blacklists):
			print_result(path, results["spam"], message)
		else:
			print_result(path, results["ok"], message)
	except:
		print_result(path, results["fail"], messages["err_open"])
	return

def print_result(path, result, message = ""):
	if (message == ""):
		print(path + " - " + result)
	else:
		print(path + " - " + result + " - " + message)

def load_blacklist(language):
	blacklist = {}
	for t in types:
		blacklist.update({t:[line.rstrip("\n") for line in open(language + "_blacklist_" + t)]})		
	return blacklist

blacklists = []
for language in languages:
	blacklists.append(load_blacklist(language))
for path in sys.argv[1:]:
	check_file(path, blacklists)
    