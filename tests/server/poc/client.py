import asyncio
import codecs


async def client_input(ftp_ip, ftp_port):
    writer = None

    try:
        # Connection + initial response
        reader, writer = await asyncio.open_connection(ftp_ip, ftp_port)
        response = await reader.readline()
        print(f"-> {response.decode()!r}")

        # Team name and first response (2 response in one)
        team_name = input("<- ")
        team_name = codecs.decode(team_name, "unicode_escape")
        writer.write(team_name.encode())
        await writer.drain()
        response = await reader.readline()
        print(f"-> {response.decode()!r}")
        response = await reader.readline()
        print(f"-> {response.decode()!r}")

        # Main loop for commands
        while True:
            cmd = input("<- ")
            cmd = codecs.decode(cmd, "unicode_escape")
            writer.write(cmd.encode())
            await writer.drain()
            for _ in range(cmd.count("\n")):
                response = await reader.readline()
                print(f"-> {response.decode()!r}")

    except asyncio.CancelledError:
        print("Cancelled")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if writer is None:
            return
        else:
            writer.close()
            await writer.wait_closed()


async def multiple_clients(ftp_ip, ftp_port, n):
    tasks = (
        asyncio.create_task(client_input(ftp_ip, ftp_port)) for _ in range(n)
    )
    await asyncio.gather(*tasks)


async def main():
    srv = "127.0.0.1", 4242

    client = asyncio.create_task(client_input(*srv))
    # await multiple_clients(*srv, 100)
    await client


if __name__ == "__main__":
    asyncio.run(main())
