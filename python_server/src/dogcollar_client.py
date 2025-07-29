import requests
from bs4 import BeautifulSoup



class DogCollarClient:
    def __init__(self, esp_32_server_url):
        self.esp_32_server_url = esp_32_server_url

    def get_file_list(self):
        response = requests.get(f"{self.esp_32_server_url}/files")
        if response.status_code == 200:
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
        response = requests.get(f"{self.esp_32_server_url}/download?file={file_name}")
        if response.status_code == 200:
            with open(file_name, 'wb') as file:
                file.write(response.content)
            return True
        else:
            print(f"Failed to download file {file_name}: {response.status_code}")
        return False

if __name__ == "__main__":
    client = DogCollarClient("http://dogcollar.local")
    files = client.get_file_list()
    print(files)