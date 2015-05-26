/*
   Copyright 2015, Google Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
package orp.dchat;

import android.content.Intent;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;


public class Login extends ActionBarActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        final Model model = Model.getModel();
        final TextView tvUserName = (TextView)findViewById(R.id.tvUserName);
        tvUserName.setText(model.getUserName());
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_login, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }


    public void setLoginButtonVisible(boolean isVisible) {
        final Button btnLogin = (Button)findViewById(R.id.btnLogin);
        btnLogin.setVisibility(isVisible ? View.VISIBLE : View.GONE);
    }

    public void setLoginProgressVisible(boolean isVisible) {
        final ProgressBar pbLogin = (ProgressBar)findViewById(R.id.pbLogin);
        pbLogin.setVisibility(isVisible ? View.VISIBLE : View.GONE);
    }

    public void doLogin(View view) {
        setLoginButtonVisible(false);
        setLoginProgressVisible(true);

        final Login loginActivity = this;

        final Model model = Model.getModel();

        // The contacts thread, used if login succeeds
        final Thread contactsThread = new Thread(new Runnable() {
            @Override
            public void run() {
                if (!model.connect())
                    return;
                if (!model.initializeContact())
                    return;
                model.listenLoop();
            }
        });

        // The login thread
        Thread loginThread = new Thread(new Runnable() {
            @Override
            public void run() {
                final boolean isSuccess = model.initialize() && model.login();

                loginActivity.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (isSuccess) {
                            setLoginButtonVisible(false);
                            setLoginProgressVisible(false);

                            Intent intent = new Intent(loginActivity, Contacts.class);
                            startActivity(intent);

                            contactsThread.start();
                        } else {
                            setLoginButtonVisible(true);
                            setLoginProgressVisible(false);
                        }
                    }
                });
            }
        });

        loginThread.start();
    }
}
