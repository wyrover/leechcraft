/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#pragma once

#include <QString>
#include <QDateTime>
#include <QtPlugin>
#include "messagebase.h"

class QObject;

namespace LeechCraft
{
namespace Azoth
{
	class ICLEntry;

	/** @brief This interface is used to represent a message.
	 *
	 * Refer to the MessageType enum for the list of possible message
	 * types that are covered by this interface.
	 *
	 * The message should not be sent upon creation, only call to Send()
	 * should trigger the sending.
	 *
	 * This interface provides only more or less basic functionality.
	 * Advanced features like delivery receipts and such, are in
	 * IAdvancedMessage.
	 *
	 * Messages implementing this interface are expected to contain only
	 * plain text bodies. If a message may also contain rich text, it
	 * should implement the IRichTextMessage interface.
	 *
	 * @sa IAdvancedMessage, IRichTextMessage.
	 */
	class IMessage : public MessageBase
	{
	public:
		virtual ~IMessage () {};

		/** @brief Represents the direction of the message.
		 */
		enum Direction
		{
			/** @brief The message is from the remote party to us.
			 */
			DIn,

			/** @brief The message is from us to the remote party.
			 */
			DOut
		};

		/** @brief Represents possible message types.
		 */
		enum MessageType
		{
			/** @brief Standard one-to-one message.
			 */
			MTChatMessage,

			/** @brief Message in a multiuser conference.
			 *
			 * This kind of message is only for messages that originate
			 * from participants and are human-generated. Thus, status
			 * changes, topic changes and such should have a different
			 * type.
			 */
			MTMUCMessage,

			/** @brief Status changes in a chat.
			 *
			 * This type of message contains information about
			 * participant's status/presence changes.
			 */
			MTStatusMessage,

			/** @brief Various events in a chat.
			 *
			 * Messages of this type are for notifying about, for
			 * example, topic changes, kicks, bans, etc. Generally,
			 * there is no other part in such messages, so the message
			 * of this type can return NULL from OtherPart().
			 */
			MTEventMessage,

			/** @brief Other.
			 */
			MTServiceMessage
		};

		/** @brief This enum is used for more precise classification of
		 * chat types messages.
		 *
		 * The messages of some particular types may have additional
		 * required properties used by the Azoth Core and other plugins
		 * to establish proper context for the events.
		 */
		enum MessageSubType
		{
			/** This message is of subtype that doesn't correspond to
			 * any other subtype of message.
			 */
			MSTOther,

			/** This message notifies about someone being just kicked.
			 */
			MSTKickNotification,

			/** This message notifies about someone being just banned.
			 */
			MSTBanNotification,

			/** @brief Represents status change of a participant in a
			 * chat or MUC room.
			 *
			 * The corresponding MessageType is MTStatusMessage.
			 *
			 * Messages of this type should have the following dynamic
			 * properties:
			 * - Azoth/Nick, with a QString representing nick of the
			 *   participant that has changed its status.
			 * - Azoth/TargetState, with a QString representing the
			 *   target state. The string is better obtained from the
			 *   State enum by the means of IProxyObject::StateToString
			 *   method.
			 * - Azoth/StatusText, with a QString representing the new
			 *   status text of the participant. May be empty.
			 */
			MSTParticipantStatusChange,

			/** @brief Represents permission changes of a participant in
			 * a chat or MUC room.
			 */
			MSTParticipantRoleAffiliationChange,

			/** @brief Notifies about participant joining to a MUC room.
			 */
			MSTParticipantJoin,

			/** @brief Notifies about participant leaving a MUC room.
			 */
			MSTParticipantLeave,

			/** @brief Notifies about participant in a MUC changing the
			 * nick.
			 */
			MSTParticipantNickChange,

			/** @brief The participant has ended the conversation.
			 */
			MSTParticipantEndedConversation,

			/** @brief Notifies about changing subject in a MUC room.
			 */
			MSTRoomSubjectChange
		};

		/** @brief Returns this message as a QObject.
		 */
		virtual QObject* GetQObject () = 0;

		/** @brief Sends the message.
		 *
		 * A message should never be sent except as the result of this
		 * method.
		 *
		 * Please note that if the other part is a MUC, it should send
		 * back this message with the "IN" direction set.
		 */
		virtual void Send () = 0;

		/** @brief Stores the message.
		 *
		 * After calling this function this message should be present in
		 * the messages list returned by ICLEntry::GetAllMessages(), but
		 * the message shouldn't be sent.
		 *
		 * @sa ICLEntry::GetAllMessages()
		 */
		virtual void Store () = 0;

		/** @brief Returns the direction of this message.
		 *
		 * @return The direction of this message.
		 */
		virtual Direction GetDirection () const = 0;

		/** @brief Returns the type of this message.
		 *
		 * @return The type of this message.
		 */
		virtual MessageType GetMessageType () const = 0;

		/** @brief Returns the subtype of this message.
		 *
		 * The subtype is used for more precise classification of
		 * messages.
		 *
		 * @return The subtype of this message
		 */
		virtual MessageSubType GetMessageSubType () const = 0;

		/** @brief Returns the CL entry from which this message is.
		 *
		 * For normal, single user chats, this should always be equal to
		 * the ICLEntry that was (and will be) used to send the message
		 * back.
		 *
		 * For multiuser chats this should be equal to the contact list
		 * representation of the participant that sent the message.
		 *
		 * The returned object must implement ICLEntry.
		 *
		 * @return The CL entry from which this message originates,
		 * implementing ICLEntry.
		 */
		virtual QObject* OtherPart () const = 0;

		/** @brief Returns the parent CL entry of this message.
		 *
		 * This is the same that OtherPart() for single user chats. For
		 * multiuser chats it should be the contact list entry
		 * representing the MUC room.
		 *
		 * By default this function calls OtherPart() and returns its
		 * result.
		 *
		 * The returned object must implement ICLEntry.
		 *
		 * @return The parent CL entry of this message, implementing
		 * ICLEntry.
		 */
		virtual QObject* ParentCLEntry () const
		{
			return OtherPart ();
		}

		/** @brief The variant of the other part.
		 *
		 * If not applicable, a null string should be returned.
		 *
		 * @return The variant of the other part.
		 */
		virtual QString GetOtherVariant () const = 0;

		/** @brief Returns the body of the message.
		 *
		 * The body is expected to be a plain text string. All '<' and
		 * '&' will be escaped.
		 *
		 * @return The body of the message.
		 */
		virtual QString GetBody () const = 0;

		/** @brief Updates the body of the message.
		 *
		 * The passed string is the plain text contents of the message.
		 *
		 * @param[in] body The new body of the message.
		 */
		virtual void SetBody (const QString& body) = 0;

		/** @brief Returns the timestamp of the message.
		 *
		 * @return The timestamp of the message.
		 */
		virtual QDateTime GetDateTime () const = 0;

		/** @brief Updates the timestamp of the message.
		 *
		 * @param[in] timestamp The new timestamp of the message.
		 */
		virtual void SetDateTime (const QDateTime& timestamp) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IMessage,
	"org.Deviant.LeechCraft.Azoth.IMessage/1.0");
