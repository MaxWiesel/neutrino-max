/*
	Movie Database - TMDb

	Copyright (C) 2015-2020 TangoCash

	License: GPLv2

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation;

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>
#include <set>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <neutrino.h>
#include <system/settings.h>
#include <system/set_threadname.h>
#include <system/debug.h>
#include <driver/screen_max.h>
#include <global.h>
#include "mdb-tmdb.h"

CTMDB *CTMDB::getInstance()
{
	static CTMDB *tmdb = NULL;
	if (!tmdb)
		tmdb = new CTMDB();
	return tmdb;
}

CTMDB::CTMDB()
{
	key = g_settings.tmdb_api_key;
	posterfile = "/tmp/tmdb.jpg";
	hintbox = NULL;
}

CTMDB::~CTMDB()
{
	cleanup();
}

void CTMDB::setTitle(std::string epgtitle)
{
	minfo.epgtitle = epgtitle;

	hintbox = new CHintBox(LOCALE_MESSAGEBOX_INFO, LOCALE_TMDB_READ_DATA);
	hintbox->paint();

	std::string lang = Lang2ISO639_1(g_settings.language);
	getMovieDetails(lang);
	if ((minfo.result < 1 || minfo.overview.empty()) && lang != "en")
		getMovieDetails("en", true);

	if (hintbox)
	{
		hintbox->hide();
		delete hintbox;
		hintbox = NULL;
	}
}

bool CTMDB::getData(std::string url, Json::Value *root)
{
	std::string answer;
	if (!getUrl(url, answer))
		return false;

	std::string errMsg = "";
	bool ok = parseJsonFromString(answer, root, &errMsg);
	if (!ok)
	{
		printf("Failed to parse JSON\n");
		printf("%s\n", errMsg.c_str());
		return false;
	}
	return true;
}

bool CTMDB::getMovieDetails(std::string lang, bool second)
{
	printf("[TMDB]: %s\n", __func__);
	Json::Value root;
	const std::string urlapi = "http://api.themoviedb.org/3/";
	std::string url	= urlapi + "search/multi?api_key=" + key + "&language=" + lang + "&query=" + encodeUrl(minfo.epgtitle);
	if (!(getData(url, &root)))
		return false;

	minfo.result = root.get("total_results", 0).asInt();
	if (minfo.result == 0)
	{
		std::string title = minfo.epgtitle;
		size_t pos1 = title.find_last_of("(");
		size_t pos2 = title.find_last_of(")");
		if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
		{
			printf("[TMDB]: second try\n");
			title.replace(pos1, pos2 - pos1 + 1, "");
			url	= urlapi + "search/multi?api_key=" + key + "&language=" + lang + "&query=" + encodeUrl(title);
			if (!(getData(url, &root)))
				return false;

			minfo.result = root.get("total_results", 0).asInt();
		}
	}
	printf("[TMDB]: results: %d\n", minfo.result);

	if (minfo.result > 0)
	{
		Json::Value elements = root["results"];
		int use_result = 0;

		if ((minfo.result > 1) && (!second))
			selectResult(elements, minfo.result, use_result);

		if (!second)
		{
			minfo.id = elements[use_result].get("id", -1).asInt();
			minfo.media_type = elements[use_result].get("media_type", "").asString();
		}
		if (minfo.id > -1)
		{
			url = urlapi + minfo.media_type + "/" + to_string(minfo.id) + "?api_key=" + key + "&language=" + lang + "&append_to_response=credits";
			if (!(getData(url, &root)))
				return false;

			minfo.overview = root.get("overview", "").asString();
			minfo.poster_path = root.get("poster_path", "").asString();
			minfo.original_title = root.get("original_title", "").asString();;
			minfo.release_date = root.get("release_date", "").asString();;
			minfo.vote_average = root.get("vote_average", "").asString();;
			minfo.vote_count = root.get("vote_count", 0).asInt();;
			minfo.runtime = root.get("runtime", 0).asInt();;
			if (minfo.media_type == "tv")
			{
				minfo.original_title = root.get("original_name", "").asString();;
				minfo.episodes = root.get("number_of_episodes", 0).asInt();;
				minfo.seasons = root.get("number_of_seasons", 0).asInt();;
				minfo.release_date = root.get("first_air_date", "").asString();;
				elements = root["episode_run_time"];
				minfo.runtimes = elements[0].asString();
				for (unsigned int i = 1; i < elements.size(); i++)
				{
					minfo.runtimes +=  + ", " + elements[i].asString();
				}
			}

			elements = root["genres"];
			minfo.genres = elements[0].get("name", "").asString();
			for (unsigned int i = 1; i < elements.size(); i++)
			{
				minfo.genres += ", " + elements[i].get("name", "").asString();
			}

			elements = root["credits"]["cast"];
			minfo.cast.clear();
			for (unsigned int i = 0; i < elements.size() && i < 10; i++)
			{
				minfo.cast +=  "  " + elements[i].get("character", "").asString() + " (" + elements[i].get("name", "").asString() + ")\n";
				// dprintf(DEBUG_NORMAL, "[TMDB]: %s (%s)\n", elements[i].get("character","").asString().c_str(), elements[i].get("name","").asString().c_str());
			}

			unlink(posterfile.c_str());
			if (hasPoster())
			{
				getBigPoster(posterfile.c_str());
				// dprintf(DEBUG_NORMAL, "[TMDB]: %s (%s) %s\n %s\n %d\n", minfo.epgtitle.c_str(), minfo.original_title.c_str(), minfo.release_date.c_str(), minfo.overview.c_str(), minfo.found);
			}
			return true;
		}
	}
	else
		return false;

	return false;
}

std::string CTMDB::getEPGText()
{
	std::string epgtext("");

	epgtext += "Vote: " + minfo.vote_average.substr(0, 3) + "/10; Votecount: " + to_string(minfo.vote_count) + "\n";
	epgtext += "\n";
	epgtext += minfo.overview + "\n";
	epgtext += "\n";
	if (minfo.media_type == "tv")
		epgtext += g_Locale->getString(LOCALE_EPGVIEWER_LENGTH) + ": " + minfo.runtimes + "\n";
	else
		epgtext += g_Locale->getString(LOCALE_EPGVIEWER_LENGTH) + ": " + to_string(minfo.runtime) + "\n";
	epgtext += g_Locale->getString(LOCALE_EPGVIEWER_GENRE) + ": " + minfo.genres + "\n";
	epgtext += g_Locale->getString(LOCALE_EPGEXTENDED_ORIGINAL_TITLE) + ": " + minfo.original_title + "\n";
	epgtext += g_Locale->getString(LOCALE_EPGEXTENDED_YEAR_OF_PRODUCTION) + ": " + minfo.release_date.substr(0, 4) + "\n";
	if (minfo.media_type == "tv")
		epgtext += "Seasons/Episodes: " + to_string(minfo.seasons) + "/" + to_string(minfo.episodes) + "\n";
	if (!minfo.cast.empty())
		epgtext += g_Locale->getString(LOCALE_EPGEXTENDED_ACTORS) + ":\n" + minfo.cast + "\n";

	return epgtext;
}

std::string CTMDB::getMovieText()
{
	std::string movietext("");

	if (!minfo.overview.empty())
	{
		movietext = minfo.overview + "\n";
		if (!minfo.cast.empty())
		{
			movietext += "\n";
			movietext += g_Locale->getString(LOCALE_EPGEXTENDED_ACTORS) + ":\n";
			movietext += minfo.cast;
		}
	}

	return movietext;
}

void CTMDB::cleanup()
{
	if (access(posterfile.c_str(), F_OK) == 0)
		unlink(posterfile.c_str());
}

void CTMDB::selectResult(Json::Value elements, int results, int &use_result)
{
	if (hintbox)
	{
		hintbox->hide();
		delete hintbox;
		hintbox = NULL;
	}

	int select = 0;

	CMenuWidget *m = new CMenuWidget(LOCALE_TMDB_READ_DATA, NEUTRINO_ICON_TMDB);
	CMenuSelectorTarget *selector = new CMenuSelectorTarget(&select);

	// we don't show introitems, so we add a separator for a smoother view
	m->addItem(GenericMenuSeparator);
	CMenuForwarder *mf;
	int counter = std::min(results, 10);
	for (int i = 0; i != counter; i++)
	{
		if (elements[i].get("media_type", "").asString() == "movie")
			mf = new CMenuForwarder(elements[i].get("title", "").asString(), true, NULL, selector, to_string(i).c_str());
		else
			mf = new CMenuForwarder(elements[i].get("name", "").asString(), true, NULL, selector, to_string(i).c_str());
		m->addItem(mf);
	}

	m->enableSaveScreen();
	m->exec(NULL, "");
	if (!m->gotAction())
		return;
	delete selector;
	delete m;
	use_result = select;
}
