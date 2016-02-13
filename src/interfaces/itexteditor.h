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

#include <QVariantMap>

class QWidget;
class QString;
class QAction;
class QColor;

namespace LeechCraft
{
	/** Possible content types a text editor could handle.
	 */
	enum class ContentType
	{
		/** HTML content. The corresponding editor should provide WYSIWYG
		 * capabilities or at least some fancy highlighting for HTML.
		 */
		HTML,

		/** Well, plain text.
		 */
		PlainText
	};

	/** @brief Enumeration for some standard editor actions.
	 *
	 * The corresponding actions may be retrieved via
	 * ITextEditor::GetEditorAction().
	 *
	 * @sa ITextEditor::GetEditorAction()
	 */
	enum class EditorAction
	{
		/** @brief Open "Find" dialog.
		 */
		Find,

		/** @brief Open "Replace" dialog.
		 */
		Replace,

		/** @brief Toggle the boldness of the selected text's font.
		 */
		Bold,

		/** @brief Toggle the italicness of the selected text's font.
		 */
		Italic,

		/** @brief Toggle underlining of the selected text's font.
		 */
		Underline,

		/** @brief Open the dialog for inserting a hyperlink.
		 */
		InsertLink,

		/** @brief Open the dialog for inserting an image.
		 */
		InsertImage,

		/** @brief Toggle between WYSIWYG and source view, if applicable.
		 */
		ToggleView
	};
}

/** @brief Interface to be implemented by editor widgets returned from
 * ITextEditor::GetTextEditor().
 *
 * HTML content can be considered as either HTML (taking all the markup
 * into account) or just plain text (without all the markup, just the
 * user-visible characters). The type parameter to GetContents() and
 * SetContents() methods toggles the way HTML content is considered.
 *
 * In general, ContentType::HTML type is used when the caller considers
 * the contents as HTML-enabled, and ContentType::PlainText otherwise.
 *
 * A text editor widget may also implement IAdvancedHTMLEditor and
 * IAdvancedPlainTextEditor if it supports the functionality required by
 * those interfaces.
 *
 * For an HTML editor to expose configurable fonts, it should implement
 * IWkFontsSettable.
 *
 * @sa IAdvancedHTMLEditor
 * @sa IAdvancedPlainTextEditor
 * @sa IWkFontsSettable
 */
class Q_DECL_EXPORT IEditorWidget
{
public:
	virtual ~IEditorWidget () {}

	/** @brief Returns the editor contents for the given type.
	 *
	 * Returns the contents of this widget according to type.
	 *
	 * For example, for a WYSIWYG text editor widget with HTML contents
	 * @code <h1>header</h1> @endcode this function should return
	 * @code header @endcode for ContentType::PlainText (prerendering
	 * the text in a sense) and @code <h1>header</h1> @endcode for
	 * ContentType::HTML.
	 *
	 * @param[in] type How contents should be interpreted and returned.
	 * @return The editor contents interpreted according to type.
	 *
	 * @sa SetContents()
	 */
	virtual QString GetContents (LeechCraft::ContentType type) const = 0;

	/** @brief Sets contents of this widget interpreted as of the given
	 * type.
	 *
	 * Sets the contents of this widget to contents according to
	 * contents. If type is ContentType::HTML the contents should be
	 * interpreted as rich text, while if type is ContentType::PlainText
	 * the contents should be interpreted as plain text. That is,
	 * @code <h1>header</h1> @endcode should be shown as a big h1 header
	 * in the first case and as is, with tags, in the second.
	 *
	 * Another example is a string like @code <p> @endcode, which should
	 * be kept as is by an HTML editor if the type is ContentType::HTML,
	 * but converted to @code &lt;p> @endcode if the type is
	 * ContentType::PlainText.
	 *
	 * @param[in] contents The new contents of this widget.
	 * @param[in] type The type of the contents.
	 *
	 * @sa GetContents()
	 */
	virtual void SetContents (QString contents, LeechCraft::ContentType type) = 0;

	/** @brief Returns a standard editor action.
	 *
	 * Returns the given standard editor action or null if no such action
	 * is available. Ownership is \em not passed to the caller.
	 *
	 * @param[in] action The standard action to return.
	 * @return The action or null if not available.
	 */
	virtual QAction* GetEditorAction (LeechCraft::EditorAction action) = 0;

	/** @brief Adds a custom action to the editor toolbar, if any.
	 *
	 * This function adds a custom action to the editor toolbar, if the
	 * widget has any. Ownershit is \em not passed to the editor. The
	 * action can be later removed by RemoveAction().
	 *
	 * @param[in] action The custom action to add.
	 *
	 * @sa RemoveAction()
	 */
	virtual void AppendAction (QAction *action) = 0;

	/** @brief Appens an empty separator action to the editor toolbar.
	 */
	virtual void AppendSeparator () = 0;

	/** @brief Removes a custom action from the editor.
	 *
	 * This function removes a custom action previously added by
	 * AppendAction().
	 *
	 * @param[in] action An action previously added by AppendAction().
	 *
	 * @sa AppendAction()
	 */
	virtual void RemoveAction (QAction *action) = 0;

	/** @brief Sets the background color of the \em editor to color.
	 *
	 * This function sets the background color of the \em editor of the
	 * given content-type to the given \em color.
	 *
	 * If the widget doesn't support the given content-type, this
	 * function does nothing.
	 *
	 * @param[in] color The new background color.
	 * @param[in] editor The editor to change color of.
	 */
	virtual void SetBackgroundColor (const QColor& color, LeechCraft::ContentType editor) = 0;

	virtual QWidget* GetWidget () = 0;

	virtual QObject* GetQObject () = 0;
protected:
	/** @brief Notifies about contents changes.
	 *
	 * This signal is emitted each time contents of this editor widget
	 * change.
	 */
	virtual void textChanged () = 0;
};

/** @brief Interface for plugins implementing a text editor component.
 *
 * If a plugin can provide a text editor widget for other plugins to use
 * it should implement this interface.
 *
 * For example, plugins like a blog client (Blogique) or mail client
 * (Snails) would use such editor widget to allow the user to write posts
 * or mails.
 */
class Q_DECL_EXPORT ITextEditor
{
public:
	virtual ~ITextEditor () {}

	/** @brief Whether this editor plugin supports editing the content type.
	 *
	 * Plain text editors should return true only for the
	 * ContentType::PlainText type, while WYSIWYG-enabled editors should
	 * return true for ContentType::HTML as well.
	 *
	 * @param[in] type The content type to query.
	 * @return Whether the plugin supports editing the given type.
	 */
	virtual bool SupportsEditor (LeechCraft::ContentType type) const = 0;

	/** @brief Creates and returns a new text editor for the given type.
	 *
	 * This function should create a new text editor widget implementing
	 * IEditorWidget for the given content type. If creation fails for
	 * some reason (like unsupported type) this function should return 0.
	 *
	 * It is generally OK to return a WYSIWYG-enabled editor for the
	 * ContentType::PlainText type as long as it supports editing plain
	 * text.
	 *
	 * @param[in] type The content type for which to create the editor.
	 * @return An editor widget implementing IEditorWidget or nullptr in
	 * case of failure.
	 */
	virtual QWidget* GetTextEditor (LeechCraft::ContentType type) = 0;
};

Q_DECLARE_INTERFACE (IEditorWidget, "org.Deviant.LeechCraft.IEditorWidget/1.0");
Q_DECLARE_INTERFACE (ITextEditor, "org.Deviant.LeechCraft.ITextEditor/1.0");
