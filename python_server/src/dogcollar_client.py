import requests
from bs4 import BeautifulSoup
from local_storage_manager import LocalStorageManager, FILE_LIST_ENDPOINT, GPX_FILES_DIR, DOWNLOAD_FILE_ENDPOINT
from gpx_converter import GPXConverter
from strava_uploader import StravaUploader
import os

HTTP_OK = 200   
DOWNLOAD_TIMEOUT = 10 


class DogCollarClient:
    def __init__(self, esp_32_server_url: str) -> None:
        self.storage_manager = LocalStorageManager()
        self.GPXConverter = GPXConverter()
        self.strava_uploader = StravaUploader()
        self.esp_32_server_url = esp_32_server_url

    def is_connected(self) -> bool:
        try:
            response = requests.get(self.esp_32_server_url, timeout=DOWNLOAD_TIMEOUT)
            return response.status_code == HTTP_OK
        except requests.exceptions.RequestException as e:
            return False
        except Exception as e:
            return None

    def get_file_list(self) -> list[str]:

        # Check if the server is connected to ESP32
        if not self.is_connected():
            #print("Cannot retrieve file list: Not connected to ESP32 server.")
            return []

        # Construct the URL for the file list endpoint
        url = f"{self.esp_32_server_url}{FILE_LIST_ENDPOINT}" #---> dogcollar.local/files

        # Make the request to retrieve the file list
        try:
            response = requests.get(url, timeout=DOWNLOAD_TIMEOUT)
            response.raise_for_status()  # Raises HTTPError for bad responses
        except requests.exceptions.HTTPError as e:
            print(f"HTTP error while retrieving file list: {e.response.status_code}")
            return []
        except requests.exceptions.RequestException as e:
            print(f"Network error while retrieving file list: {str(e)}")
            return []
        except Exception as e:
            print(f"Unexpected error while retrieving file list: {str(e)}")
            return []

        # Return file list if successful
        try:
            print("Got the file list.")
            return self.parse_file_list(response.content)
        except Exception as e:
            print(f"Failed to parse file list: {str(e)}")
            return []

    def parse_file_list(self, response_content: str) -> list[str]:
        soup = BeautifulSoup(response_content, "html.parser")
        file_names = []
        for link in soup.find_all("a"):
            href = link.get("href")
            if href and DOWNLOAD_FILE_ENDPOINT in href:
                file_names.append(link.text)
        return file_names

    def download_file(self, file_name: str) -> bool:

        # Ensure the file name is valid
        if not file_name or not isinstance(file_name, str):
            #print("Invalid or empty file name provided.")
            return False

        # Check if the file already exists locally
        if self.storage_manager.file_exists(file_name):
            #print(f"File '{file_name}' already exists locally. Skipping download.")
            return False

        # Construct the URL for the download endpoint
        url = f"{self.esp_32_server_url}/{DOWNLOAD_FILE_ENDPOINT}{file_name}" #---> dogcollar.local/download?file=FILENAME

        # Check if the client is connected before attempting to download
        if not self.is_connected():
            return False

        # Make the request to download the file
        try:
            response = requests.get(url, timeout=DOWNLOAD_TIMEOUT)
            response.raise_for_status()
        except requests.exceptions.HTTPError as e:
            print(f"HTTP error while downloading '{file_name}': {e.response.status_code}")
            return False
        except requests.exceptions.RequestException as e:
            print(f"Network error while downloading '{file_name}': {e}")
            return False
        except Exception as e:
            print(f"Unexpected error while downloading '{file_name}': {e}")
            return False

        # Save the downloaded file locally
        self.storage_manager.save_file_locally(file_name, response.content)
        print(f"Downloaded '{file_name}' successfully.")
        return True
    



