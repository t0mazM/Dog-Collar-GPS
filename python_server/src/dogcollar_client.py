import requests
from bs4 import BeautifulSoup
import local_storage_manager

HTTP_OK = 200   
DOWNLOAD_TIMEOUT = 10 
class DogCollarClient:
    def __init__(self, esp_32_server_url):
        self.storage_manager = local_storage_manager.LocalStorageManager()
        self.esp_32_server_url = esp_32_server_url

    def is_connected(self):
        try:
            response = requests.get(self.esp_32_server_url, timeout=DOWNLOAD_TIMEOUT)
            return response.status_code == HTTP_OK
        except requests.exceptions.RequestException as e:
            print(f"Error connecting to ESP32 server: {e}")
            return False

    def get_file_list(self):
        response = requests.get(f"{self.esp_32_server_url}/files")
        if response.status_code == HTTP_OK:
            return self.parse_file_list(response.content)
        else:
            raise print(f"Failed to retrieve file list: {response.status_code}")

    def parse_file_list(self, response_content):
        soup = BeautifulSoup(response_content, "html.parser")
        file_names = []
        for link in soup.find_all("a"):
            href = link.get("href")
            if href and "download?file=" in href:
                file_names.append(link.text)
        return file_names

    def download_file(self, file_name):
        if self.storage_manager.file_exists(file_name):
            print(f"File '{file_name}' already exists locally. Skipping download.")
            return True

        url = f"{self.esp_32_server_url}/download?file={file_name}"

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
    file_names = client.get_file_list()

    for file_name in file_names:

        # 2) Download each file 
        client.download_file(file_name)

        # 3) Open file
        file = client.storage_manager.get_file_locally(file_name)

        # 4) Convert to gpx
        # process file - convert to gpx
        # upload to strava
        pass