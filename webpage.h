#include <string>
#include <set>
#include <fstream>

class WebPage {
 public:
  	/**
	 * Default constructor
   	 */
 	WebPage(std::string n);

	/**
	 * Destructor
	 */
	~WebPage();

	/**
	 * Adds word to words set
	 */
	void addWord(const std::string &word);

	/**
	 * Adds outgoing link to outgoing set 
	 */
	void addOutgoingLink(const std::string &link);

	/**
	 * Adds incoming link to incoming set
	 */
	void addIncomingLink(const std::string &link);

	/**
	 * Prints outgoing links
	 */
	void printOutgoingLinks(std::ofstream &output);

	/**
	 * Prints incoming links
	 */
	void printIncomingLinks(std::ofstream &output);

	/**
	 * Returns filename of WebPage
	 */
	std::string getName() const;

	/**
	 * Returns outgoing links of WebPage
	 */
	std::set<std::string> getOutgoingLinks() const;

	/**
	 * Returns incoming links of WebPage
	 */
	std::set<std::string> getIncomingLinks() const;

	/**
	 * Returns page rank of WebPage
	 */
	double getPageRank() const;

	/**
	 * Sets page rank of WebPage
	 */
	void setPageRank(double rank);

 private:
 	/**
	 * Data members
	 */
 	std::string name;
 	std::set<std::string> words;
 	std::set<std::string> outgoing;
 	std::set<std::string> incoming;
 	double pageRank;
};