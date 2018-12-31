/***************************************************************************
    File                 : AbstractScript.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Knut Franke (knut.franke@gmx.de)
    Description          : A chunk of scripting code.

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "AbstractScript.h"
#include "AbstractScriptingEngine.h"

/**
 * \class AbstractScript
 * \brief A chunk of scripting code.
 *
 * AbstractScript objects represent a chunk of code, possibly together with local
 * variables. The code may be changed and executed multiple times during the
 * lifetime of an object.
 */

AbstractScript::AbstractScript(AbstractScriptingEngine *engine, QString code, QObject *context, QString name)
	: m_engine(engine), m_code(std::move(code)), m_name(std::move(name)), m_context(context) {
	m_engine->incref();
}

AbstractScript::~AbstractScript()
{
	m_engine->decref();
}

/**
 * \brief Return the code that will be executed/evaluated when calling exec() or eval()
 */
const QString AbstractScript::code() const {
	return m_code;
}

/**
 * \brief Return the context in which the code is to be executed.
 */
const QObject *AbstractScript::context() const {
	return m_context;
}

/**
 * \brief Like QObject::name, but with unicode support.
 */
const QString AbstractScript::name() const {
	return m_name;
}

/**
 * \brief Return whether errors / exceptions are to be emitted or silently ignored
 */
bool AbstractScript::emitErrors() const {
	return m_emit_errors;
}

/**
 * \brief Append to the code that will be executed when calling exec() or eval()
 */
void AbstractScript::addCode(const QString &code) {
	m_code.append(code);
	m_compiled = notCompiled;
	emit codeChanged();
}

/**
 * \brief Set the code that will be executed when calling exec() or eval()
 */
void AbstractScript::setCode(const QString &code) {
	m_code=code;
	m_compiled = notCompiled;
	emit codeChanged();
}

/**
 * \brief Set the context in which the code is to be executed.
 */
void AbstractScript::setContext(QObject *context) {
	m_context = context;
	m_compiled = notCompiled;
}

/**
 * \brief Like QObject::setName, but with unicode support.
 */
void AbstractScript::setName(const QString &name) {
	m_name = name;
	m_compiled = notCompiled;
}

/**
 * \brief Set whether errors / exceptions are to be emitted or silently ignored
 */
void AbstractScript::setEmitErrors(bool value) {
	m_emit_errors = value;
}

/**
 * \brief Compile the content of #m_code.
 *
 * \param for_eval whether the code is to be evaluated later on (as opposed to executed)
 * \return True iff compilation was successful or the implementation doesn't support compilation.
 */
bool AbstractScript::compile(bool for_eval) {
	Q_UNUSED(for_eval);
	return true;
}

/**
 * \fn QVariant AbstractScript::eval()
 * \brief Evaluate #m_code, returning QVariant() on an error / exception.
 */

/**
 * \fn bool AbstracScript::exec()
 * \brief Execute #m_code, returning false on an error / exception.
 */

/**
 * \fn void AbstractScript::codeChanged()
 * \brief This is emitted whenever the code to be executed by exec() and eval() is changed.
 */

/**
 * \fn void AbstractScript::error(const QString &message, const QString &scriptName, int lineNumber)
 * \brief signal an error condition / exception
 */

/**
 * \fn void AbstractScript::print(const QString &output)
 * \brief output generated by the code
 */

