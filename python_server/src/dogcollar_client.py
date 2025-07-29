import requests
from bs4 import BeautifulSoup
import local_storage_manager

HTTP_OK = 200   
class DogCollarClient:
    def __init__(self, esp_32_server_url):
        self.storage_manager = local_storage_manager.LocalStorageManager()
        self.esp_32_server_url = esp_32_server_url

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
        try:
            response = requests.get(f"{self.esp_32_server_url}/download?file={file_name}", timeout=10)
            if response.status_code == HTTP_OK:
                self.storage_manager.save_file_locally(file_name, response.content)
                print(f"Downloaded {file_name}")
                return True
            else:
                print(f"Failed to download file {file_name}: {response.status_code}")
                return False
        except requests.exceptions.RequestException as e:
            print(f"Connection error while downloading {file_name}: {e}")
            return False

if __name__ == "__main__":
    client = DogCollarClient("http://dogcollar.local")
    file_names = client.get_file_list()
    print(file_names)

    for file_name in file_names:
        client.download_file(file_name)
        pass