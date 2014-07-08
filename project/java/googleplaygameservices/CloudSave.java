/*
 * Copyright (C) 2013 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.example.games.basegameutils;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import com.google.android.gms.common.api.GoogleApiClient;

public class CloudSave implements GameHelper.GameHelperListener {

	// The game helper object. This class is mainly a wrapper around this object.
	protected GameHelper mHelper;

	// Requested clients. By default, that's just the games client.
	public int mRequestedClients = GameHelper.CLIENT_SNAPSHOT;

	MainActivity parent;

	/** Constructs a BaseGameActivity with default client (GamesClient). */
	public CloudSave(MainActivity p)
	{
		Log.i("SDL", "CloudSave: initializing");
		parent = p;
		setRequestedClients(GameHelper.CLIENT_SNAPSHOT);
		getGameHelper().setup(this);
	}

	/**
	 * Sets the requested clients. The preferred way to set the requested clients is
	 * via the constructor, but this method is available if for some reason your code
	 * cannot do this in the constructor. This must be called before onCreate or getGameHelper()
	 * in order to have any effect. If called after onCreate()/getGameHelper(), this method
	 * is a no-op.
	 *
	 * @param requestedClients A combination of the flags CLIENT_GAMES, CLIENT_PLUS
	 *		   and CLIENT_APPSTATE, or CLIENT_ALL to request all available clients.
	 */
	public void setRequestedClients(int requestedClients) {
		mRequestedClients = requestedClients;
	}

	public GameHelper getGameHelper() {
		if (mHelper == null) {
			mHelper = new GameHelper(parent, mRequestedClients);
		}
		return mHelper;
	}

	public void onStart() {
		mHelper.onStart(parent);
	}

	public void onStop() {
		mHelper.onStop();
	}

	public void onActivityResult(int request, int response, Intent data) {
		mHelper.onActivityResult(request, response, data);
	}

	public void onSignInSucceeded() {
		Log.i("SDL", "CloudSave: onSignInSucceeded()");
	}

	public void onSignInFailed() {
		Log.i("SDL", "CloudSave: onSignInFailed()");
	}

	public GoogleApiClient getApiClient() {
		return mHelper.getApiClient();
	}

	public boolean isSignedIn() {
		return mHelper.isSignedIn();
	}

	public void beginUserInitiatedSignIn() {
		mHelper.beginUserInitiatedSignIn();
	}

	public void signOut() {
		mHelper.signOut();
	}

	public void showAlert(String message) {
		mHelper.makeSimpleDialog(message).show();
	}

	public void showAlert(String title, String message) {
		mHelper.makeSimpleDialog(title, message).show();
	}

	public void enableDebugLog(boolean enabled) {
		mHelper.enableDebugLog(enabled);
	}

	public String getInvitationId() {
		return mHelper.getInvitationId();
	}

	public void reconnectClient() {
		mHelper.reconnectClient();
	}

	public boolean hasSignInError() {
		return mHelper.hasSignInError();
	}

	public GameHelper.SignInFailureReason getSignInError() {
		return mHelper.getSignInError();
	}
}
