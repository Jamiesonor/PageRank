#include "webpage.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <vector>
#include <iostream>

using namespace std;

// Read in the config file and initialize parameters
void readConfig (ifstream& config, ifstream& index, ifstream& query, ofstream& output, double& epsilon, int& steps) {
	// Iterate through the config file
	string line;
	while (getline(config, line)) {
		
		// Ignore comments
		if (line.find("#") != string::npos) {
			line.erase(line.find('#'));
		}

		// Replace '=' with whitespace
		if (line.find("=") != string::npos) {
			replace(line.begin(), line.end(), '=', ' ');
		}

		stringstream ss(line);
		string str;
		
		// Iterate through line
		while (ss >> str) {
			// Parse config parameters
			if (str == "INDEX_FILE") {
				string fileName;
				ss >> fileName;
				index.open(fileName);
			} else if (str == "QUERY_FILE") {
				string fileName;
				ss >> fileName;
				query.open(fileName);
			} else if (str == "OUTPUT_FILE") {
				string fileName;
				ss >> fileName;
				output.open(fileName);
			} else if (str == "RESTART_PROBABILITY") {
				ss >> epsilon;
			} else if (str == "STEP_NUMBER") {
				ss >> steps;
			}
		}
	}
}

// Read in index files
void readIndex (ifstream &index, set<WebPage*> &webPages) {
	string fileName;
	// Parse through index file
	while (getline(index, fileName)) {
    	if (fileName != "") {
    		// Create new WebPage
			WebPage* webPage = new WebPage(fileName);
    		webPages.insert(webPage);
    	}
    }
}

// Parse webpage contents
void parseWebPages (set<WebPage*> &webPages, map<string, set<WebPage*> > &mapWordToPage) {
	// Iterate through WebPages
	for (set<WebPage*>::iterator it = webPages.begin(); it != webPages.end(); it++) {
    	WebPage* page = *it;
    	ifstream input(page->getName());
    	string line;
    	// Loop through WebPage content line-by-line
    	while (getline(input, line)) {
    		string word;
    		string fileLink;
    		bool link = false;
    		// Check for valid characters
    		for (unsigned int i = 0; i < line.length(); i++) {
    			// Parsing an outgoing WebPage link
    			if (link) {
    				// End of link
    				if (line[i] == ')') {
    					// Add outgoing link to current page
    					page->addOutgoingLink(fileLink);
    					// Add incoming link to linked page
    					for (set<WebPage*>::iterator iter = webPages.begin(); iter != webPages.end(); iter++) {
    						WebPage* linkPage = *iter;
    						if (linkPage->getName() == fileLink) {
    							linkPage->addIncomingLink(page->getName());
    						}
    					}
    					link = false;
    					fileLink = "";
    				} else {
    					fileLink += line[i];
    				}
    			} else {
    				// Parse WebPage words
    				if ((line[i] >= 48 && line[i] <= 57) 
	    					|| (line[i] >= 65 && line[i] <= 90) 
	    					|| (line[i] >= 97 && line[i] <= 122)) {
    					char c = line[i];
    					c = tolower(c);
	    				word += c;
	    			} else {
	    				if (word != "") {
	    					page->addWord(word);
	    					mapWordToPage[word].insert(page);
	    					word = "";
	    				}
	    				if (line[i] == '(') {
	    					link = true;
	    				}
	    			}
    			}
    		}
    		// Add final word of line if it isn't followed by an invalid character
    		if (word != "") {
    			page->addWord(word);
    			mapWordToPage[word].insert(page);
    		}
    	}
    }
}

// Print the content of a webpage
void printPage (string &fileName, ofstream &output) {
	ifstream input(fileName);
	// Check valid fileName
	if (input.fail()) {
		output << "Invalid query" << endl;
	} else {
		// Print out file line-by-line, ignoring links
		output << fileName << endl;
		string line;
		while (getline(input, line)) {
			bool link = false;
			for (unsigned int i = 0; i < line.length(); i++) {
				if (link) {
					if (line[i] == ')')
						link = false;
				} else {
					if (line[i] == '(')
						link = true;
					else
						output << line[i];
				}
			}
			output << endl;
		}
	}
}

// Print either the incoming or outgoing links of a webpage
void printLinks (string &fileName, ofstream &output, set<WebPage*> &webPages, string &commandType) {
	bool fileExists = false;
	// Find specified WebPage
	for (set<WebPage*>::iterator it = webPages.begin(); it != webPages.end(); it++) {
		WebPage* page = *it;
		if (page->getName() == fileName) {
			fileExists = true;
			// Check Incoming vs Outgoing
			if (commandType == "INCOMING")
				page->printIncomingLinks(output);
			else
				page->printOutgoingLinks(output);
			break;
		}
	}
	if (!fileExists) {
		output << "Invalid query" << endl;
	}
}

// Search for webpages containing the specified word
set<WebPage*> searchSingleWord (string &word, map<string, set<WebPage*> > &mapWordToPage) {
	// Convert to lowercase
	for (unsigned int i = 0; i < word.length(); i++) {
		word[i] = tolower(word[i]);
	}

	// Return map with search word as key and set of all WebPages containing search word as value
	map<string, set<WebPage*> >::iterator itWord = mapWordToPage.find(word);

	set<WebPage*> wordPages;

	if (itWord != mapWordToPage.end()) {
		wordPages = itWord->second;
	}

	return wordPages; // If word isn't found, set returns empty
}

// Returns set of WebPages resulting from intersection operation (AND)
set<WebPage*> intersectWebPage (string& word1, string& word2, stringstream &ss, map<string, set<WebPage*> > &mapWordToPage) {
	set<WebPage*> andPages;

	// Convert to lowercase
	for (unsigned int i = 0; i < word1.length(); i++) {
		word1[i] = tolower(word1[i]);
	}
	for (unsigned int i = 0; i < word2.length(); i++) {
		word2[i] = tolower(word2[i]);
	}

	// Return maps of search term to WebPages of both search words
	map<string, set<WebPage*> >::iterator itWord1 = mapWordToPage.find(word1);
	map<string, set<WebPage*> >::iterator itWord2 = mapWordToPage.find(word2);

	if (itWord1 != mapWordToPage.end() && itWord2 != mapWordToPage.end()) {
		set<WebPage*> word1Pages = itWord1->second;
		set<WebPage*> word2Pages = itWord2->second;

		// intersection returns set of pages with word1 and word2
		if (word1Pages.size() <= word2Pages.size()) {
			for (set<WebPage*>::iterator it1 = word1Pages.begin(); it1 != word1Pages.end(); it1++) {
				WebPage* page = *it1;
				set<WebPage*>::iterator it2 = word2Pages.find(page);
				if (it2 != word2Pages.end()) {
					andPages.insert(page);
				}
			}
		} else {
			for (set<WebPage*>::iterator it2 = word2Pages.begin(); it2 != word2Pages.end(); it2++) {
				WebPage* page = *it2;
				set<WebPage*>::iterator it1 = word1Pages.find(page);
				if (it1 != word1Pages.end()) {
					andPages.insert(page);
				}
			}
		}
	}		

	string word;

	// Perform intersection with andSet and following word sets
	while (ss >> word) {
		for (unsigned int i = 0; i < word.length(); i++) {
			word[i] = tolower(word[i]);
		}
		map<string, set<WebPage*> >::iterator itWord = mapWordToPage.find(word);
		if (itWord != mapWordToPage.end()) {
			set<WebPage*> wordPages = itWord->second;
			for (set<WebPage*>::iterator itAnd = andPages.begin(); itAnd != andPages.end(); itAnd++) {
				WebPage* page = *itAnd;
				set<WebPage*>::iterator it = wordPages.find(page);
				if (it == wordPages.end()) {
					andPages.erase(page);
				}
			}
		} else {
			// if the word is not found in any page, there is no intersection whatsoever
			for (set<WebPage*>::iterator itAnd = andPages.begin(); itAnd != andPages.end(); itAnd++) {
				WebPage* page = *itAnd;
				andPages.erase(page);
			}
		}
	}

	return andPages;
}

// Returns set of WebPages resulting from union operation (OR)
set<WebPage*> unionWebPage (stringstream &ss, map<string, set<WebPage*> > &mapWordToPage) {
	set<WebPage*> orPages;
	string word;
	while (ss >> word) {
		for (unsigned int i = 0; i < word.length(); i++) {
			word[i] = tolower(word[i]);
		}
		map<string, set<WebPage*> >::iterator itWord = mapWordToPage.find(word);
		if (itWord != mapWordToPage.end()) {
			set<WebPage*> wordPages = itWord->second;
			for (set<WebPage*>::iterator it = wordPages.begin(); it != wordPages.end(); it++) {
				WebPage* page = *it;
				orPages.insert(page);
			}
		}
	}

	return orPages;
}

// Add all incoming and outgoing pages to set of pages that satisfy search query
set<WebPage*> expandCandidateSet (set<WebPage*> &webPages, set<WebPage*> &candidateSet) {
	set<WebPage*> expandedCandidateSet = candidateSet;
	// Loop through pages that satisfy search query
	for (set<WebPage*>::iterator it = candidateSet.begin(); it != candidateSet.end(); it++) {
		WebPage* page = *it;
		set<string> outgoing = page->getOutgoingLinks();
		set<string> incoming = page->getIncomingLinks();
		// Loop through outgoing pages
		for (set<string>::iterator itOut = outgoing.begin(); itOut != outgoing.end(); itOut++) {
			string linkName = *itOut;
			for (set<WebPage*>::iterator iter = webPages.begin(); iter != webPages.end(); iter++) {
				WebPage* webPage = *iter;
				if (webPage->getName() == linkName) {
					// Add outgoing pages to candidate set
					expandedCandidateSet.insert(webPage);
				}
			}
		}
		// Loop through incoming pages
		for (set<string>::iterator itInc = incoming.begin(); itInc != incoming.end(); itInc++) {
			string linkName = *itInc;
			for (set<WebPage*>::iterator iter = webPages.begin(); iter != webPages.end(); iter++) {
				WebPage* webPage = *iter;
				if (webPage->getName() == linkName) {
					// Add incoming pages to candidate set
					expandedCandidateSet.insert(webPage);
				}
			}
		}
	}

	return expandedCandidateSet;
}

// Checks which page has a higher rank
bool comparePageRank (WebPage* page1, WebPage* page2) {
	return (page1->getPageRank() > page2->getPageRank());
}

// PageRank algorithm: evaluate link structure of graph and rank each page
void pageRank (set<WebPage*> &candidateSet, double epsilon, int steps) {
	// Store webpages in adjacency matrix
	int vertexCount = candidateSet.size();
	vector<vector<double>> adjMatrix(vertexCount, vector<double>(vertexCount));

	// Loop through candidate set and initialize adjacency matrix
	int i = 0;
	for (set<WebPage*>::iterator it = candidateSet.begin(); it != candidateSet.end(); it++) {
		WebPage* page = *it;
		set<string> outgoing = page->getOutgoingLinks();
		adjMatrix[i][i] = 1.0 / (outgoing.size() + 1); // Add self-loop
		for (set<string>::iterator itOut = outgoing.begin(); itOut != outgoing.end(); itOut++) {
			string linkName = *itOut;
			int j = 0;
			for (set<WebPage*>::iterator iter = candidateSet.begin(); iter != candidateSet.end(); iter++) {
				WebPage* webPage = *iter;
				if (webPage->getName() == linkName) {
					adjMatrix[j][i] = 1.0 / (outgoing.size() + 1);
 				}
				j++;
			}
		}
		i++;
	}

	vector<double> pageRanks;

	// Initialize pageRanks to 1 / n
	for (set<WebPage*>::iterator it = candidateSet.begin(); it != candidateSet.end(); it++) {
		WebPage* page = *it;
		double pageRank = 1.0 / candidateSet.size();
		page->setPageRank(pageRank);
		pageRanks.push_back(pageRank);
	}

	// Calculate PageRank values iteratively with matrix multiplication
	for (int t = 1; t <= steps; t++) {
		int i = 0;
		for (set<WebPage*>::iterator it = candidateSet.begin(); it != candidateSet.end(); it++) {
			WebPage* page = *it;
			double newRank = 0;
			for (unsigned int j = 0; j < candidateSet.size(); j++) {
				newRank += adjMatrix[i][j] * pageRanks[j];
			}

			newRank *= (1 - epsilon);
			newRank += (epsilon) * (1.0 / candidateSet.size());

			page->setPageRank(newRank);
			i++;
		}

		// Store PageRank values for next iteration
		i = 0;
		for (set<WebPage*>::iterator it = candidateSet.begin(); it != candidateSet.end(); it++) {
			WebPage* page = *it;
			pageRanks[i] = page->getPageRank();
			i++;
		}
	}
}

// Execute the search commands (queries)
void processQueries (ifstream &query, ofstream &output, set<WebPage*> &webPages, map<string, set<WebPage*> > &mapWordToPage, double epsilon, int steps) {
	string command;
	// Parse search query
    while (getline(query, command)) {
    	if (command != "") {
    		stringstream ss(command);
	    	string commandType;
	    	ss >> commandType;
	    	// Determine command type
	    	if (ss.peek() != EOF) {
	    		// Display WebPage contents
	    		if (commandType == "PRINT") {
	    			
	    			string fileName;
	    			ss >> fileName;
	    			printPage(fileName, output);

	    		// Print incoming or outgoing links of WebPage
	    		} else if (commandType == "INCOMING" || commandType == "OUTGOING") {

	    			string fileName;
	    			ss >> fileName;
	    			printLinks(fileName, output, webPages, commandType);

	    		// Search for WebPages containing both search words
	    		} else if (commandType == "AND") {
	    			string word1;
	    			string word2;

	    			ss >> word1;

	    			// Check if only one word is searched
	    			if (ss.peek() == EOF) {
	    				// Return pages containing single search word
	    				set<WebPage*> wordPages = searchSingleWord(word1, mapWordToPage);
						if (wordPages.size() == 0)	// There are no pages containing search word
							output << "0" << endl;
						else {
							set<WebPage*> candidateSet = expandCandidateSet(webPages, wordPages);

							pageRank(candidateSet, epsilon, steps);

							// Sort the candidate set in order of PageRank
							vector<WebPage*> sortedPages;
							for (set<WebPage*>::iterator it = candidateSet.begin(); it != candidateSet.end(); it++) {
								WebPage* page = *it;
								sortedPages.push_back(page);
							}

							sort(sortedPages.begin(), sortedPages.end(), comparePageRank);

							// Output sorted pages from single word search
							output << sortedPages.size() << endl;
							for (unsigned int i = 0; i < sortedPages.size(); i++) {
								output << sortedPages[i]->getName() << endl;
							}
						}

	    			} else {
		    			ss >> word2;	// Read in second search word
		    			
		    			// Return WebPages containing both search words with set intersection
		    			set<WebPage*> andPages = intersectWebPage(word1, word2, ss, mapWordToPage);

		    			set<WebPage*> candidateSet = expandCandidateSet(webPages, andPages);

		    			pageRank(candidateSet, epsilon, steps);

		    			// Sort the candidate set in order of PageRank
		    			vector<WebPage*> sortedPages;
						for (set<WebPage*>::iterator it = candidateSet.begin(); it != candidateSet.end(); it++) {
							WebPage* page = *it;
							sortedPages.push_back(page);
						}

		    			sort(sortedPages.begin(), sortedPages.end(), comparePageRank);

		    			// Print sorted set of pages resulting from AND command
		    			output << sortedPages.size() << endl;
						for (unsigned int i = 0; i < sortedPages.size(); i++) {
							output << sortedPages[i]->getName() << endl;
						}
	    			}

	    		// Search for WebPages containing at least one of the search words
	    		} else if (commandType == "OR") {
	    			
	    			// Return WebPages containing at least one of the search words with set union
	    			set<WebPage*> orPages = unionWebPage(ss, mapWordToPage);

	    			set<WebPage*> candidateSet = expandCandidateSet(webPages, orPages);

	    			pageRank(candidateSet, epsilon, steps);

	    			// Sort the candidate set in order of PageRank
	    			vector<WebPage*> sortedPages;
					for (set<WebPage*>::iterator it = candidateSet.begin(); it != candidateSet.end(); it++) {
						WebPage* page = *it;
						sortedPages.push_back(page);
					}

	    			sort(sortedPages.begin(), sortedPages.end(), comparePageRank);

	    			// Print sorted set of pages resulting from OR command
	    			output << sortedPages.size() << endl;
					for (unsigned int i = 0; i < sortedPages.size(); i++) {
						output << sortedPages[i]->getName() << endl;
					}
	    		} else {
	    			output << "Invalid query" << endl;
	    		}
	    	} else {
	    		// Search for the single word
				set<WebPage*> wordPages = searchSingleWord(command, mapWordToPage);
				if (wordPages.size() == 0)	// No WebPages contain the search word
					output << "0" << endl;
				else {
					set<WebPage*> candidateSet = expandCandidateSet(webPages, wordPages);

					pageRank(candidateSet, epsilon, steps);

					// Sort the candidate set in order of PageRank
					vector<WebPage*> sortedPages;
					for (set<WebPage*>::iterator it = candidateSet.begin(); it != candidateSet.end(); it++) {
						WebPage* page = *it;
						sortedPages.push_back(page);
					}

					sort(sortedPages.begin(), sortedPages.end(), comparePageRank);

					// Output sorted pages from single word search
					output << sortedPages.size() << endl;
					for (unsigned int i = 0; i < sortedPages.size(); i++) {
						output << sortedPages[i]->getName() << endl;
					}
				}
	    	}
    	} else {
    		output << "Invalid query" << endl;
    	}
    }
}

int main (int argc, char* argv[])
{
	ifstream config;
	ifstream index;
	ifstream query;
	ofstream output;
	double epsilon;
	int steps;

	// Check for config file
	if (argc < 2)
		config.open("config.txt");
	else
		config.open(argv[1]);

	// Check if config file exists
	if (config.fail()) {
		cerr << "Couldn't open the config file" << endl;
		return 1;
	}

	// Read config file
	readConfig(config, index, query, output, epsilon, steps);

    set<WebPage*> webPages;
    map<string, set<WebPage*> > mapWordToPage;

    // Read index file
    readIndex(index, webPages);

    // Parse WebPage files for links and words
    parseWebPages(webPages, mapWordToPage);

    // Execute the user's commands and search queries
    processQueries(query, output, webPages, mapWordToPage, epsilon, steps);

    // Deallocate WebPage memory
    for (set<WebPage*>::iterator it = webPages.begin(); it != webPages.end(); it++)
    {
    	WebPage* page = *it;
    	delete page;
    }

    return 0;
}