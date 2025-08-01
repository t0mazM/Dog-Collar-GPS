import requests
import json
import time

from strava_credentials import STRAVA_ACCESS_TOKEN, STRAVA_CLIENT_ID, STRAVA_CLIENT_SECRET, STRAVA_AUTH_CODE

STRAVA_TOKENS_JSON = 'strava_tokens.json'

def auth_strava_account():
    # You need to authenticate your Strava account - grand certain acces to your tokens
    # Follow this tutorial: https://medium.com/swlh/using-python-to-connect-to-stravas-api-and-analyse-your-activities-dummies-guide-5f49727aac86
    # or do this:
    # Open this URL in your browser:
    # https://www.strava.com/oauth/authorize?client_id=INSERT_YOUR_CLIENT_ID&response_type=code&redirect_uri=http://localhost/exchange_token&approval_prompt=force&scope=profile:read_all,activity:read_all,activity:write
    # Replace INSERT_YOUR_CLIENT_ID with your actual client ID from Strava
    # After you authorize the app, you will be redirected to the redirect_uri (the page won't load, but you will see the code in the URL). 
    # Copy the code from the URL and paste it below in the STRAVA_AUTH_CODE variable.
    # Then run this script to get your access token.
    # This function will work only once, but you need to authenticate your account only once.
    response = requests.post(
                        url = 'https://www.strava.com/oauth/token',
                        data = {
                                'client_id': STRAVA_CLIENT_ID,
                                'client_secret': STRAVA_CLIENT_SECRET,
                                'code': STRAVA_AUTH_CODE,  # Get it by following this tutorial: https://medium.com/swlh/using-python-to-connect-to-stravas-api-and-analyse-your-activities-dummies-guide-5f49727aac86 , I know, I am a dummy :)
                                'grant_type': 'authorization_code'
                                }
                    )
    #Save tokens to file
    strava_tokens = response.json()
    with open(STRAVA_TOKENS_JSON, 'w') as file:
        json.dump(strava_tokens, file)


class StravaUploader:
    def __init__(self):
        self.tokens = self.load_tokens()

    def load_tokens(self):
        try:
            with open(STRAVA_TOKENS_JSON, 'r') as file:
                return json.load(file)
        except FileNotFoundError:
            print("Strava tokens file not found. Please authenticate your Strava account first using the auth_strava_account function.")
            return None
        
    def refresh_access_token(self):
        if not self.tokens:
            self.tokens = self.load_tokens()

        if self.tokens['expires_at'] > time.time():
            print("Access token is still valid. No need to refresh it.")
            return None
        
        print("Access token has expired. Refreshing the token...")
        response = requests.post(
            url='https://www.strava.com/oauth/token',
            data={
                'client_id': STRAVA_CLIENT_ID,
                'client_secret': STRAVA_CLIENT_SECRET,
                'grant_type': 'refresh_token',
                'refresh_token': self.tokens['refresh_token']
            }
        )
        
        if response.status_code == 200:
            new_tokens = response.json()
            with open(STRAVA_TOKENS_JSON, 'w') as file:
                json.dump(new_tokens, file)
            print("Access token refreshed successfully.")
            self.tokens = new_tokens
            return None
        else:
            print(f"Failed to refresh access token: {response.status_code} -> {response.text}")
            return None
        
    def upload_gpx_file(self, gpx_file_path):
        self.refresh_access_token()
        if not self.tokens:
            print("No valid Strava tokens available.")
            return None

        access_token = self.tokens.get("access_token")
        url = "https://www.strava.com/api/v3/uploads"
        headers = {"Authorization": f"Bearer {access_token}"}
        files = {
            "file": (gpx_file_path, open(gpx_file_path, "rb"), "application/gpx+xml"),
            "data_type": (None, "gpx"),
        }

        response = requests.post(url, headers=headers, files=files)
        if response.status_code == 201 or response.status_code == 200:
            print("GPX file uploaded to Strava. Response:", response.json())
            return response.json()
        else:
            print(f"Failed to upload GPX file: {response.status_code} -> {response.text}")
            return None
        


if __name__ == "__main__":



    strava_uploader = StravaUploader()
    strava_uploader.refresh_access_token()

    # Example usage: Upload a GPX file
    gpx_file_path = "gpx_files/FARTLEK_15_1min.gpx"  # Replace with your actual GPX file path
    response = strava_uploader.upload_gpx_file(gpx_file_path)