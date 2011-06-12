/*
 * message_mgr.h
 * Declares classes and functions for handling of program messages in multiple languages.
 *
 * Copyright (c) 2004-2006, Dale McCoy.
 * Copyright (c) 2006, Dan Masek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _RENUM_MESSAGE_MGR_H_INCLUDED_
#define _RENUM_MESSAGE_MGR_H_INCLUDED_

#include <cstdarg>
#include <string>

#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code
#endif

#include <map>

#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif

#include "language_mgr.h"
#include "messages.h"

/*! Collection mapping language IDs to strings. */
typedef std::map<RenumLanguageId,std::string> lang2str_map;

/*! Encapsulates a NFORenum message. */
class MessageData {
public:
	/*! Creates a new instance of MessageData with the specified properties.
		\param props_ Initial properties of the message.
	*/
	MessageData(char props_ = 0);

	/*! Creates a new instance of MessageData with the specified properties
		and text in the default language.
		\param props_ Initial properties of the message.
		\param default_text Text of the message in the default language.
	*/
	MessageData(char props_, const std::string& default_text);

	/*! Check whether this message is meant for the console.
		\return true if the message is meant for the console.
	*/
	bool IsConsoleMessage() const {
		return !(props & NO_CONSOLE);
	}

	/*! Check whether this message is meant to make a comment.
		\return true if the message is supposed to make a comment.
	*/
	bool IsMakeComment() const {
		return (props & MAKE_COMMENT);
	}

	/*! Output the message to the appropriate stream.
		\param prefix Message prefix
		\param ap Message parameter list
		\return The composed message
	*/
	std::string Display(const std::string& prefix, std::va_list& ap) const;

	/*! Get the text of this message in the current language.
		If the text is not available in the current language,
		get text in the default language. If even that fails
		return a fallback string.
		\return Text of the message, or MessageMgr::UNDEFINED_TEXT if not available.
	*/
	const std::string& GetText() const;

	/*! Get the properties of this message.
		\return Message properties
	*/
	char GetProps() const { return props; }

	/*! Set message text for the specified language.
		\param lang Language to set the text for
		\param text Text of the message
	*/
	void SetText(RenumLanguageId lang, const std::string& text);

private:
	/*! Renders the message in the current language with given optional prefix.
		\param prefix Optional message prefix
		\return Rendered message
	*/
	std::string GetMessage(const std::string& prefix = "") const;

	/*! A string inserted between comment delimiter and the beginning of a message. */
	const static std::string commentPrefix;
	lang2str_map textMap; /*!< Contains message texts for each language. */
	char props; /*!< Message properties */
};

/*! Collection mapping message IDs to message data. */
typedef std::map<RenumMessageId,MessageData> msgid2data_map;
/*! Collection mapping extra text IDs to extra text data. */
typedef std::map<RenumExtraTextId,lang2str_map> extid2data_map;

/*! Manages program messages and extra texts for various supported languages. */
class MessageMgr {
	SINGLETON(MessageMgr);
public:
	const static MessageData UNKNOWN_MESSAGE; /*!< Fallback message data. */
	const static std::string UNDEFINED_TEXT; /*!< Fallback message text. */

	/*!	Get the data for the specified message ID in the current language.
		\param i Message ID.
		\return Reference to message data for the given ID.
			If the ID is unknown, a reference to UNKNOWN_MESSAGE is returned.
	*/
	const MessageData& GetMessageData(RenumMessageId i) const;

	/*!	Get the data for the specified message ID in the current language.
		\param i Extra text ID.
		\return Extra text, or MessageMgr::UNDEFINED_TEXT if not available.
	*/
	const std::string& GetExtraText(RenumExtraTextId i) const;

	/*!	Adds new message data for the specified message ID.
		\param i Message ID to add.
		\param props Properties of this message.
		\return true if successful.
			If message data for the speficied ID already exists,
			this function returns false.
	*/
	bool AddMessage(RenumMessageId i, char props);

	/*! Sets the message text for the specified message ID and language.
		\param i Message ID
		\param lang Language this particular text is in
		\param text Text of the message
		\return true if successful. If no such message ID exists returns false.
	*/
	bool SetMessageText(RenumMessageId i, RenumLanguageId lang, const std::string& text);

	/*!	Set the extra text for the specifified extra text ID and language.
		\param i Extra text ID
		\param lang Language this particular text is in
		\param text The extra text
	*/
	void SetExtraText(RenumExtraTextId i, RenumLanguageId lang, const std::string& text);

private:
	/*! Initialize message data and properties. */
	void InitMessages();

	/*! Initialize message texts for all supported languages. */
	void InitMessageTexts();

	/*! Initialize extra text data for all supported languages. */
	void InitExtraTexts();

	msgid2data_map msgDataMap; /*!< Message data for each message ID. */
	extid2data_map extraTextMap; /*!< Extra text data for each extra text ID. */
};

#endif // _RENUM_MESSAGE_MGR_H_INCLUDED_
