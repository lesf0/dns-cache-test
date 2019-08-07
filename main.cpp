#include "dnscache.h"

#include <random>
#include <cstdio>
#include <cctype>
#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <chrono>
#include <exception>

std::string randomIPv4()
{
	char buf[17];

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<int> dist(0, 255);

	snprintf(buf, 17, "%hhu.%hhu.%hhu.%hhu",
			 (uint8_t)dist(gen), (uint8_t)dist(gen), (uint8_t)dist(gen), (uint8_t)dist(gen));

	return buf;
}

std::string randomDomain()
{
	static const char* words[] = {"Improve","Trust","Immediately","Discover","Profit","Learn","Know","Understand","Powerful","Best","Win","More","Bonus","Exclusive","Extra","You","Free","Health","Guarantee","New","Proven","Safety","Money","Now","Today","Results","Protect","Help","Easy","Amazing","Latest","Extraordinary","Worst","Ultimate","Hot","First","Big","Anniversary","Premiere","Basic","Complete","Save","Plus","Create","Secret","Inspires","Take","Help","Promote","Increase","Create","Discover","Anonymous","Authentic","Backed","Bestselling","Certified","Endorsed","Guaranteed","Ironclad","Lifetime","Moneyback","Official","Privacy","Protected","Proven","Recessionproof","Refund","Research","Results","Secure","Tested","Verify","Unconditional","Suddenly","Now","Announcing","Introducing","Improvement","Amazing","Sensational","Remarkable","Revolutionary","Startling","Miracle","Magic","Offer","Quick","Easy","Wanted","Challenge","Compare","Bargain","Hurry","You","Free","Because","Instantly","New"};
	static const char* zones[] = {".af",".ax",".al",".dz",".as",".ad",".ao",".ai",".aq",".ag",".ar",".am",".aw",".ac",".au",".at",".az",".bs",".bh",".bd",".bb",".eus",".by",".be",".bz",".bj",".bm",".bt",".bo",".bq",".an",".nl",".ba",".bw",".bv",".br",".io",".vg",".bn",".bg",".bf",".mm",".bi",".kh",".cm",".ca",".cv",".cat",".ky",".cf",".td",".cl",".cn",".cx",".cc",".co",".km",".cd",".cg",".ck",".cr",".ci",".hr",".cu",".cw",".cy",".cz",".dk",".dj",".dm",".do",".tl",".tp",".ec",".eg",".sv",".gq",".er",".ee",".et",".eu",".fk",".fo",".fm",".fj",".fi",".fr",".gf",".pf",".tf",".ga",".gal",".gm",".ps",".ge",".de",".gh",".gi",".gr",".gl",".gd",".gp",".gu",".gt",".gg",".gn",".gw",".gy",".ht",".hm",".hn",".hk",".hu",".is",".in",".id",".ir",".iq",".ie",".im",".il",".it",".jm",".jp",".je",".jo",".kz",".ke",".ki",".kw",".kg",".la",".lv",".lb",".ls",".lr",".ly",".li",".lt",".lu",".mo",".mk",".mg",".mw",".my",".mv",".ml",".mt",".mh",".mq",".mr",".mu",".yt",".mx",".md",".mc",".mn",".me",".ms",".ma",".mz",".mm",".na",".nr",".np",".nl",".nc",".nz",".ni",".ne",".ng",".nu",".nf",".nc",".tr",".kp",".mk",".mp",".no",".om",".pk",".pw",".ps",".pa",".pg",".py",".pe",".ph",".pn",".pl",".pt",".pr",".qa",".ro",".ru",".rw",".re",".bq",".an",".bl",".gp",".fr",".sh",".kn",".lc",".mf",".gp",".fr",".pm",".vc",".ws",".sm",".st",".sa",".sn",".rs",".sc",".sl",".sg",".bq",".an",".nl",".sx",".an",".sk",".si",".sb",".so",".so",".za",".gs",".kr",".ss",".es",".lk",".sd",".sr",".sj",".sz",".se",".ch",".sy",".tw",".tj",".tz",".th",".tg",".tk",".to",".tt",".tn",".tr",".tm",".tc",".tv",".ug",".ua",".ae",".uk",".us",".vi",".uy",".uz",".vu",".va",".ve",".vn",".wf",".eh",".ma",".ye",".zm",".zw"};

	static const size_t wordsCount = sizeof(words)/sizeof(std::string);
	static const size_t zonesCount = sizeof(zones)/sizeof(std::string);

	std::string res;

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<int> wordsDist(0, wordsCount-1);
	std::uniform_int_distribution<int> zonesDist(0, zonesCount-1);

	for (int i=rand()%3+3; i>0; i--)
	{
		res += words[wordsDist(gen)];
	}

	res += zones[zonesDist(gen)];

	return res;
}

std::chrono::milliseconds randomDelay()
{
	using namespace std::chrono_literals;

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<int> dist(50, 100);

	return dist(gen) * 1ms;
}

std::mutex output_mutex;
void worker()
{
	std::this_thread::sleep_for(randomDelay());

	auto ip=randomIPv4();
	auto domain=randomDomain();

	auto& cache = DNSCache<2048>::instance();

	cache.update(domain, ip);

	std::this_thread::sleep_for(randomDelay());

	std::string ip_resolved;

	try {
		ip_resolved = cache.resolve(domain);
	}
	catch (std::exception& e)
	{
		ip_resolved = "<no value>";
	}

	std::lock_guard<std::mutex> lock(output_mutex);
	if (ip_resolved == ip)
	{
		std::cout << "OK! " << domain << " resolves to " << ip << std::endl;
	}
	else
	{
		std::cout << "Error! " << domain << " resolves to " << ip_resolved << " instead of " << ip << std::endl;
	}
}

int main()
{
	std::vector<std::thread> threads;

	for(int i=0; i<3000; ++i)
	{
		threads.emplace_back(worker);
	}

	for(auto& thread:threads)
	{
		thread.join();
	}
}