from pathlib import Path
from PIL import Image, ImageDraw

BASE_DIR = Path(__file__).resolve().parent
LINK_ICON_PATH = BASE_DIR / "link.png"
UNLINK_ICON_PATH = BASE_DIR / "unlink.png"

def create_fallback_icon(width=64, height=64, color1=(128, 0, 0), color2=(255, 255, 255)):
    image = Image.new("RGBA", (width, height), color1)
    draw = ImageDraw.Draw(image)
    draw.rectangle((width // 4, height // 4, width * 3 // 4, height * 3 // 4), fill=color2)
    return image

def load_icon(path, fallback_color1, fallback_color2):
    try:
        if path.exists():
            with Image.open(path) as img:
                return img.convert("RGBA")
    except Exception:
        pass

    return create_fallback_icon(64, 64, fallback_color1, fallback_color2)

def load_icons():
    image_link = load_icon(LINK_ICON_PATH, (0, 128, 0), (255, 255, 255))
    image_unlink = load_icon(UNLINK_ICON_PATH, (128, 0, 0), (255, 255, 255))
    return image_link, image_unlink
