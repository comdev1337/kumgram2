#!/usr/bin/env python3
import json
import sys
import os

def get_video_attr(attributes):
    for attr in attributes:
        if 'w' in attr and 'h' in attr and 'duration' in attr:
            return attr
    return None

def format_duration(seconds):
    if seconds is None: return "N/A"
    mins = int(seconds // 60)
    secs = seconds % 60
    return f"{mins:02d}:{secs:05.2f} ({seconds:.2f}s)"

def print_doc(doc, label):
    mime = doc.get('mime_type', 'unknown')
    # Filter for actual video files
    if not mime.startswith('video/'):
        return

    attr = get_video_attr(doc.get('attributes', []))
    if not attr:
        return

    w = attr.get('w', '?')
    h = attr.get('h', '?')
    dur = attr.get('duration')
    codec = attr.get('video_codec') or "none/original"
    size_mb = doc.get('size', 0) / (1024 * 1024)

    print(f"--- {label} ---")
    print(f"  Resolution: {w}x{h} ({h}p)")
    print(f"  Duration:   {format_duration(dur)}")
    print(f"  Codec:      {codec}")
    print(f"  Mime:       {mime}")
    print(f"  File Size:  {size_mb:.2f} MB")
    print()

def main():
    if len(sys.argv) < 2:
        print("Usage: ./parse_video_json.py <file.json>")
        sys.exit(1)

    path = sys.argv[1]
    if not os.path.exists(path):
        print(f"Error: {path} not found")
        sys.exit(1)

    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    media = data.get('media', {})
    doc = media.get('document')

    if not doc:
        print("No media document found in JSON.")
        return

    # 1. Main Document
    print_doc(doc, "ATTRIBUTES (Original)")

    # 2. Alt Documents
    alts = media.get('alt_documents', [])
    if alts:
        print(f"--- ALT DOCUMENTS ({len(alts)} entries) ---")
        print()
        for i, alt in enumerate(alts):
            print_doc(alt, f"Alt #{i+1}")
    else:
        print("No alt_documents found.")

if __name__ == "__main__":
    main()
