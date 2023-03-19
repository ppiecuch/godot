/**************************************************************************/
/*  Decoder.java                                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.commons.codec;

/**
 * <p>Provides the highest level of abstraction for Decoders.
 * This is the sister interface of {@link Encoder}.  All
 * Decoders implement this common generic interface.</p>
 *
 * <p>Allows a user to pass a generic Object to any Decoder
 * implementation in the codec package.</p>
 *
 * <p>One of the two interfaces at the center of the codec package.</p>
 *
 * @author Apache Software Foundation
 * @version $Id: Decoder.java 797690 2009-07-24 23:28:35Z ggregory $
 */
public interface Decoder {
	/**
	 * Decodes an "encoded" Object and returns a "decoded"
	 * Object.  Note that the implementation of this
	 * interface will try to cast the Object parameter
	 * to the specific type expected by a particular Decoder
	 * implementation.  If a {@link ClassCastException} occurs
	 * this decode method will throw a DecoderException.
	 *
	 * @param pObject an object to "decode"
	 *
	 * @return a 'decoded" object
	 *
	 * @throws DecoderException a decoder exception can
	 * be thrown for any number of reasons.  Some good
	 * candidates are that the parameter passed to this
	 * method is null, a param cannot be cast to the
	 * appropriate type for a specific encoder.
	 */
	Object decode(Object pObject) throws DecoderException;
}
