/* $Id: MamdaErrorListener.cs,v 1.1.40.5 2012/08/24 16:12:11 clintonmcdowell Exp $
 *
 * OpenMAMA: The open middleware agnostic messaging API
 * Copyright (C) 2011 NYSE Technologies, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

using System;

namespace Wombat
{
	/// <summary>
	/// MamdaErrorListener defines an interface for handling error
	/// notifications for a MamdaSubscription.
	/// </summary>
	public interface MamdaErrorListener
	{
		/// <summary>
		/// Provide a callback to handle errors.  The severity is intended
		/// as a hint to indicate whether the error is recoverable.
		/// </summary>
		/// <param name="subscription">The subscription which received the update.</param>
		/// <param name="severity">The severity of the error.</param>
		/// <param name="errorCode">The errorCode. (<code>MamdaErrorCode</code>)</param>
		/// <param name="errorMessage">The stringified version of the error.</param>
		void onError(
			MamdaSubscription subscription,
			MamdaErrorSeverity severity,
			MamdaErrorCode errorCode,
			string errorMessage);
	}
}
