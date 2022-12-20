# Ground Station Padding Oracle

Some ground stations will be 'up for grabs' and teams can temporarily gain access to these ground stations.

One subset of ground stations will be accessible via a plaintext password token.

The players have access to a token service that provides two functionalities.
- Token distribution
- Token identification

The token distribution function provides users with all available tokens, however it encypts them for security purposes

The token identification service allows users to provide it with an encrypted token and it will identify which groundstation it is for

# Solution

The token identification service uses a poorly implemented AES256 scheme which serves as a padding oracle. By modifying the tokens from the distribution service the players can use the identification service to decrypt the tokens without the keys.

Run the solver using the following:

```bash
python3 solve.py --hostname localhost --port 13100
```

Change localhost to the IP address of the server running the challenge