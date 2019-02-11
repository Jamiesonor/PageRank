#include "webpage.h"

using namespace std;

/**
 * Default constructor
 */
WebPage::WebPage(string n) : name(n) {

}

/**
 * Destructor
 */
WebPage::~WebPage() {

}

/**
 * Adds word to words set
 */
void WebPage::addWord(const string &word) {
	words.insert(word);
}

/**
 * Adds outgoing link to outgoing set
 */
void WebPage::addOutgoingLink(const string &link) {
	outgoing.insert(link);
}

/**
 * Adds incoming link to incoming set
 */
void WebPage::addIncomingLink(const string &link) {
	incoming.insert(link);
}

/**
 * Prints outgoing links
 */
void WebPage::printOutgoingLinks(ofstream &output) {
	output << outgoing.size() << endl;
	for (set<string>::iterator it = outgoing.begin(); it != outgoing.end(); it++) {
		string link = *it;
		output << link << endl;
	}
}

/**
 * Prints incoming links
 */
void WebPage::printIncomingLinks(ofstream &output) {
	output << incoming.size() << endl;
	for (set<string>::iterator it = incoming.begin(); it != incoming.end(); it++) {
		string link = *it;
		output << link << endl;
	}
}

/**
 * Returns filename of WebPage
 */
string WebPage::getName() const {
	return name;
}

/**
 * Returns outgoing links of WebPage
 */
set<string> WebPage::getOutgoingLinks() const {
	return outgoing;
}

/**
 * Returns incoming links of WebPage
 */
set<string> WebPage::getIncomingLinks() const {
	return incoming;
}

/**
 * Returns page rank of WebPage
 */
double WebPage::getPageRank() const {
	return pageRank;
}

/**
 * Sets page rank of WebPage
 */
void WebPage::setPageRank(double rank) {
	pageRank = rank;
}