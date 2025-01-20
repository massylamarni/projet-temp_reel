
## Live Application

[https://node-base-five.vercel.app/](https://node-base-five.vercel.app/)

## Available API Endpoints

- **POST**: `/api/post/gas`
- **POST**: `/api/post/temperature`
- **POST**: `/api/post/rfid` (Special Behavior)
  - Posting an RFID value opens the door automatically.
  - Posting a value of `"0"` signifies that the door is closed.
 
## Example Usage
```bash
curl -X POST https://node-base-five.vercel.app/api/post/rfid \
  -H "Content-Type: application/json" \
  -d '{"data": "0"}'
