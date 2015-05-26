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
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;


public class Contacts extends ActionBarActivity {
    private ArrayAdapter<Model.Contact> contactsAdapter = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_contacts);

        final Model model = Model.getModel();

        final Contacts contactsActivity = this;
        model.setContactsActivity(contactsActivity);

        final TextView tvUserName = (TextView)findViewById(R.id.tvUserName);
        tvUserName.setText(model.getUserName());

        final ListView lvContacts = (ListView)findViewById(R.id.lvContacts);
        contactsAdapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, android.R.id.text1, model.getContacts());
        lvContacts.setAdapter(contactsAdapter);
        lvContacts.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Model.Contact c = (Model.Contact)lvContacts.getItemAtPosition(position);
                if (c.channelIsOpen()) {
                    Intent intent = new Intent(contactsActivity, Conversation.class);
                    startActivity(intent);
                }
            }
        });
    }

    public void notifyContactsChange() {
        if (null != contactsAdapter)
            contactsAdapter.notifyDataSetChanged();
    }

    @Override
    public void onBackPressed() {
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_contacts, menu);
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
}
