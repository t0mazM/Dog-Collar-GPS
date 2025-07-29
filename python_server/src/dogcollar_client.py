import requests
from bs4 import BeautifulSoup



class DogCollarClient:
    def __init__(self, esp_32_server_url):
        self.esp_32_server_url = esp_32_server_url

    def get_file_list(self):
        response = requests.get(f"{self.esp_32_server_url}/files")
        soup = BeautifulSoup(response.content, "html.parser")
        file_names = []
        for link in soup.find_all("a"):
            href = link.get("href")
            if href and "download?file=" in href:
                file_names.append(link.text)
        return file_names

if __name__ == "__main__":
    client = DogCollarClient("http://dogcollar.local")
    files = client.get_file_list()
    print(files)