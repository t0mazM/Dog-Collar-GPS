import requests
from bs4 import BeautifulSoup
from local_storage_manager import LocalStorageManager
from gpx_converter import GPXConverter

HTTP_OK = 200   
DOWNLOAD_TIMEOUT = 10 
FILE_LIST_ENDPOINT = "/files"

class DogCollarClient:
    def __init__(self, esp_32_server_url):
        self.storage_manager = LocalStorageManager()
        self.GPXConverter = GPXConverter()
        self.esp_32_server_url = esp_32_server_url

    def is_connected(self):
        try:
            response = requests.get(self.esp_32_server_url, timeout=DOWNLOAD_TIMEOUT)
            return response.status_code == HTTP_OK
        except requests.exceptions.RequestException as e:
            return False
        
    def get_file_list(self):
        if not self.is_connected():
            print("Cannot retrieve file list: Not connected to ESP32 server.")
            return []

        url = f"{self.esp_32_server_url}{FILE_LIST_ENDPOINT}"

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

        try:
            return self.parse_file_list(response.content)
        except Exception as e:
            print(f"Failed to parse file list: {str(e)}")
            return []

    def parse_file_list(self, response_content):
        soup = BeautifulSoup(response_content, "html.parser")
        file_names = []
        for link in soup.find_all("a"):
            href = link.get("href")
            if href and "download?file=" in href:
                file_names.append(link.text)
        return file_names

    def download_file(self, file_name):

        # Ensure the file name is valid
        if not file_name or not isinstance(file_name, str):
            print("Invalid or empty file name provided.")
            return False

        # Check if the file already exists locally
        if self.storage_manager.file_exists(file_name):
            print(f"File '{file_name}' already exists locally. Skipping download.")
            return False

        url = f"{self.esp_32_server_url}/download?file={file_name}"

        # Check if the client is connected before attempting to download
        if not self.is_connected():
            return False

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

        
        self.storage_manager.save_file_locally(file_name, response.content)
        print(f"Downloaded '{file_name}' successfully.")
        return True
    
if __name__ == "__main__":
    client = DogCollarClient("http://dogcollar.local")

    # 1) Get the list of files 
    while True:
        file_names = client.get_file_list()

        for file_name in file_names:

            # 2) Download each file 
            if not client.download_file(file_name):
                # continue commented for debugging
                pass

            # 3) Open file
            file = client.storage_manager.get_file_locally(file_name)

            # 4) Convert to gpx
            client.GPXConverter.convert_to_gpx(file_name, file)



            # 5)upload to strava
            pass