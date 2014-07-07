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

/**
 * Example base class for games. This implementation takes care of setting up
 * the API client object and managing its lifecycle. Subclasses only need to
 * override the @link{#onSignInSucceeded} and @link{#onSignInFailed} abstract
 * methods. To initiate the sign-in flow when the user clicks the sign-in
 * button, subclasses should call @link{#beginUserInitiatedSignIn}. By default,
 * this class only instantiates the GoogleApiClient object. If the PlusClient or
 * AppStateClient objects are also wanted, call the BaseGameActivity(int)
 * constructor and specify the requested clients. For example, to request
 * PlusClient and GamesClient, use BaseGameActivity(CLIENT_GAMES | CLIENT_PLUS).
 * To request all available clients, use BaseGameActivity(CLIENT_ALL).
 * Alternatively, you can also specify the requested clients via
 * @link{#setRequestedClients}, but you must do so before @link{#onCreate}
 * gets called, otherwise the call will have no effect.
 *
 * @author Bruno Oliveira (Google)
 */
public class CloudSave implements GameHelper.GameHelperListener {

	// The game helper object. This class is mainly a wrapper around this object.
	protected GameHelper mHelper;

	// We expose these constants here because we don't want users of this class
	// to have to know about GameHelper at all.
	public static final int CLIENT_GAMES = GameHelper.CLIENT_GAMES;
	public static final int CLIENT_APPSTATE = GameHelper.CLIENT_APPSTATE;
	public static final int CLIENT_PLUS = GameHelper.CLIENT_PLUS;
	public static final int CLIENT_ALL = GameHelper.CLIENT_ALL;

	// Requested clients. By default, that's just the games client.
	public int mRequestedClients = CLIENT_GAMES;

	private final static String TAG = "BaseGameActivity";
	public boolean mDebugLog = false;

	MainActivity parent;

	/** Constructs a BaseGameActivity with default client (GamesClient). */
	public CloudSave(MainActivity p)
	{
		parent = p;
		setRequestedClients(CLIENT_GAMES);
		getGameHelper().setup(this);
		/*
		// Add the Drive API and scope to the builder:
		GoogleApiClient.Builder builder = helper.getApiClientBuilder();
		GoogleApiClient.Builder builder = new GoogleApiClient.Builder(parent, this, this);
		builder.addScope(Drive.SCOPE_APPFOLDER);
		builder.addApi(Drive.API);
		*/
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
			mHelper.enableDebugLog(mDebugLog);
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
		// TODO
	}

	public void onSignInFailed() {
		// TODO
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
		mDebugLog = true;
		if (mHelper != null) {
			mHelper.enableDebugLog(enabled);
		}
	}

	public void enableDebugLog(boolean enabled, String tag) {
		enableDebugLog(enabled);
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
