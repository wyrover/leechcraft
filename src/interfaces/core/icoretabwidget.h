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

#ifndef INTERFACES_CORE_ICORETABWIDGET_H
#define INTERFACES_CORE_ICORETABWIDGET_H

#include <QTabBar>
#include <QVariant>

class QObject;
class QWidget;
class QIcon;
class QMenu;

/** @brief This interface is used to represent LeechCraft's core tab widget
 *
 * This interface is for communication with the core tab widget.
 */
class Q_DECL_EXPORT ICoreTabWidget
{
public:
	virtual ~ICoreTabWidget () {}

	/** @brief Returns the pointer to tab widget as a QObject.
	 *
	 * You can connect to signals of this class via the object returned
	 * from this function, for example.
	 *
	 * @return The core tab widget as a QObject.
	*/
	virtual QObject* GetQObject () = 0;

	/** @brief Returns the number of widgets associated with tabs.
	 *
	 * @return pages count.
	*/
	virtual int WidgetCount () const = 0;

	/** @brief Returns the tab page at index position index or 0.
	 * if the index is out of range.
	 *
	 * @param[in] index tab index.
	 * @return the page at inted position.
	 */
	virtual QWidget* Widget (int index) const = 0;

	/** @brief Returns the index of the given page.
	 *
	 * @param[in] page page to find.
	 * @return the index of that page, or -1 if not found.
	 */
	virtual int IndexOf (QWidget *page) const = 0;

	/** @brief Returns the tab menu for the given tab index.
	 *
	 * Ownership of the menu goes to the caller — it's his responsibility
	 * to delete the menu when done.
	 *
	 * @param[in] index tab index.
	 * @return tab menu for that index, or null if index is invalid.
	 */
	virtual QMenu* GetTabMenu (int index) = 0;

	/** @brief Returns the list of actions witch always shows in context menu.
	 * of the tab
	 *
	 * @return list of actions.
	 */
	virtual QList<QAction*> GetPermanentActions () const = 0;

	/** @brief Returns the text of the tab at position index,
	 * or an empty string if index is out of range.
	 *
	 * @param[in] index tab index.
	 * @return specified tab text.
	 */
	virtual QString TabText (int index) const = 0;

	/** @brief Sets the text of the tab at position index to text.
	 * of the tabs
	 *
	 * @param[in] index tab index.
	 * @param[in] text new tab text.
	 */
	virtual void SetTabText (int index, const QString& text) = 0;

	/** @brief Returns the icon of the tab at position index,
	 * or a null icon if index is out of range.
	 *
	 * @param[in] index tab index.
	 * @return specified tab icon.
	 */
	virtual QIcon TabIcon (int index) const = 0;

	/** @brief Returns the widget set a tab index and position
	 * or 0 if one is not set.
	 *
	 * @param[in] index tab index.
	 * @param[in] position position of widget.
	 * @return widget in tab.
	 */
	virtual QWidget* TabButton (int index, QTabBar::ButtonPosition position) const = 0;

	/** @brief Returns the position of close button
	 *
	 * @return close button position.
	 */
	virtual QTabBar::ButtonPosition GetCloseButtonPosition () const = 0;

	/** @brief Sets tab closable.
	 *
	 * @param[in] index tab index.
	 * @param[in] closable set tab closable.
	 * @param[in] closeButton set close button.
	 */
	virtual void SetTabClosable (int index, bool closable, QWidget *closeButton = 0) = 0;

	/** @brief Returns the index of the tab bar's visible tab.
	 *
	 * @return tab index.
	 */
	virtual int CurrentIndex () const = 0;

	/** @brief Moves the item at index position from to index position to.
	 *
	 * @param[in] from source position.
	 * @param[in] to destination position.
	 */
	virtual void MoveTab (int from, int to) = 0;

	/** @brief Sets the current tab index to specified index.
	 *
	 * @param[in] index new tab index.
	 */
	virtual void setCurrentTab (int index) = 0;

	/** @brief Sets the current tab index to specified associated widget.
	 *
	 * @param[in] widget page.
	 */
	virtual void setCurrentWidget (QWidget *widget) = 0;

	/** @brief Returns the previous active widget if it exists.
	 *
	 * @return previous widget.
	 */
	virtual QWidget* GetPreviousWidget () const = 0;
protected:
	/** @brief This signal is emitted after new tab was inserted.
	 *
	 * @param[out] index The index of new tab.
	 *
	 * @sa GetQObject()
	 */
	virtual void tabInserted (int index) = 0;

	/** @brief This signal is emitted when the tab widget's current tab changes.
	 * The new current has the given index, or -1 if there isn't a new one.
	 *
	 * @param[out] index The index of current tab.
	 *
	 * @sa GetQObject()
	 */
	virtual void currentChanged (int index) = 0;

	/** @brief This signal is emitted when tab at from moves to position to.
	 *
	 * @param[out] from Previous index of the tab that has just moved.
	 * @param[out] to The new position of the tab.
	 *
	 * @sa GetQObject()
	 */
	virtual void tabWasMoved (int from, int to) = 0;
};

Q_DECLARE_INTERFACE (ICoreTabWidget, "org.Deviant.LeechCraft.ICoreTabWidget/1.0");

#endif
