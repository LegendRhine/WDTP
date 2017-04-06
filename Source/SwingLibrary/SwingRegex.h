/*
  ==============================================================================

    SwingRegex.h
    Created: 7 Apr 2017 2:19:59am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef SWINGREGEX_H_INCLUDED
#define SWINGREGEX_H_INCLUDED

#include <regex>
#include "JuceHeader.h"

/** Encapsulate the regex operations based on chrisboy2000.
*/
struct SwingRegex
{    
    /** Searches a string and returns a StringArray with all matches.
        it can specify and index of a capture group. if not, the entire match will be used. */
    static StringArray search (const String& wildcard, 
                               const String& stringToTest, 
                               int indexInMatch = 0)
    {
        try
        {
            StringArray searchResults;

            std::regex includeRegex (wildcard.toStdString());
            std::string xAsStd = stringToTest.toStdString();
            std::sregex_iterator it (xAsStd.begin(), xAsStd.end(), includeRegex);
            std::sregex_iterator it_end;

            while (it != it_end)
            {
                std::smatch result = *it;
                StringArray matches;

                for (auto x : result)
                    matches.add (String (x));

                if (indexInMatch < matches.size()) 
                    searchResults.add (matches[indexInMatch]);

                ++it;
            }

            return searchResults;
        }

        catch (std::regex_error e)
        {
            DBG (e.what());
            return StringArray();
        }
    }

    //=================================================================================================
    /** Returns the first match of the given wildcard in the test string. 
        The first entry will be the whole match, followed by capture groups. */
    static StringArray getFirstMatch (const String& wildcard, 
                                      const String& stringToTest)
    {
        try
        {
            std::regex reg (wildcard.toStdString());
            std::string s (stringToTest.toStdString());
            std::smatch match;

            if (std::regex_search (s, match, reg))
            {
                StringArray sa;

                for (auto x : match)
                    sa.add (String (x));

                return sa;
            }

            return StringArray();
        }

        catch (std::regex_error e)
        {
            jassertfalse;
            DBG (e.what());
            return StringArray();
        }
    }

    //=================================================================================================
    /** Checks if the given string matches the regex wildcard. */
    static bool matchesWildcard (const String& wildcard, 
                                 const String& stringToTest)
    {
        try
        {
            std::regex reg (wildcard.toStdString());
            return std::regex_search (stringToTest.toStdString(), reg);
        }

        catch (std::regex_error e)
        {
            DBG (e.what());
            return false;
        }
    }

    //=================================================================================================
    /**  */
    static Array<StringArray> findSubstringsThatMatchWildcard (const String& regexWildCard,
                                                               const String& stringToTest)
    {
        Array<StringArray> matches;
        String remainingText = stringToTest;
        StringArray m = getFirstMatch (regexWildCard, remainingText);

        while (m.size() != 0 && m[0].length() != 0)
        {
            remainingText = remainingText.fromFirstOccurrenceOf (m[0], false, false);
            matches.add (m);
            m = getFirstMatch (regexWildCard, remainingText);
        }

        return matches;
    }

};


#endif  // SWINGREGEX_H_INCLUDED

