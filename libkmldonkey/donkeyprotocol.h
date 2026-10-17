/* -*- c++ -*-
 *
 * donkeyprotocol.h
 *
 * Copyright (C) 2003 Petter Stokke <ummo@hellokitty.com>
 * Copyright (C) 2003 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2006 Christian Muehlhaeuser <chris@chris.de>
 * Copyright (C) 2009 Gioacchino Mazzurco <gmazzurco89@gmail.com>
 * Copyright (C) 2009 Aleksey Markelov <markelovai@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef __libkmldonkey_donkeyprotocol_h__
#define __libkmldonkey_donkeyprotocol_h__

#include <QString>
#include <QMap>
#include <QObject>
#include <QAbstractSocket>

#include "protocoliface.h"
#include "roominfo.h"
#include "donkeytypes.h"

#include <kmldonkey_export.h>

// Maximum version of GUI protocol supported
#define MAX_PROTOCOL_VERSION 33

// Minimum version of GUI protocol supported
#define MIN_PROTOCOL_VERSION 25

class DonkeyOption;
class DonkeySocket;
//!
/*!
 * Used for sendConsoleMessage() to be able to send console
 * messages and evaluate the result for the command.
 */
class ConsoleCallbackInterface
{
public:
    virtual ~ConsoleCallbackInterface() { }
    virtual void callback(const QString& cmd, const QString& res) = 0;
};

class FileInfo;
class ServerInfo;
class Network;
class ClientInfo;
class ShareInfo;
class SearchInfo;
class ResultInfo;
class SearchQuery;

typedef QHash<int, FileInfo *> FileInfoHash;
typedef QHash<int, ShareInfo *> ShareInfoHash;
typedef QHash<int, ClientInfo *> ClientInfoHash;
typedef QHash<int, Network *> NetworkHash;
typedef QHash<int, ServerInfo *> ServerInfoHash;
typedef QHash<int, SearchInfo *> SearchInfoHash;
typedef QHash<int, ResultInfo *> ResultInfoHash;
typedef QHash<int, RoomInfo *> RoomInfoHash;

//! A high level abstraction of the mldonkey GUI protocol.
/*!
 * DonkeyProtocol wraps around a DonkeySocket object, and handles all
 * aspects of passing messages back and forth through it. It provides
 * signals and slots for linking your application to the mldonkey core
 * without having to deal with the protocol itself.
 */
class KMLDONKEY_EXPORT DonkeyProtocol : public ProtocolInterface

{
    Q_OBJECT
    Q_ENUMS(FromCore)
    Q_ENUMS(ToCore)
public:

    //! Opcodes from core messages.
    enum FromCore {
        CoreProtocol = 0,
        Options_info,
        DefineSearches = 3,
        Result_info,
        Search_result,
        Search_waiting,
        File_info_v1,
        File_downloaded_v1,
        File_update_availability,
        File_add_source, // 10
        Server_busy,
        Server_user,
        Server_state,
        Server_info_v1,
        Client_info,
        Client_state,
        Client_friend,
        Client_file,
        Console,
        Network_info, // 20
        User_info,
        Room_info,
        Room_message,
        Room_add_user,
        Client_stats_v1,
        Server_info,
        MessageFromClient,
        ConnectedServers,
        DownloadFiles_v1,
        DownloadFiles_v2, // 30
        Room_info_v2,
        Room_remove_user,
        Shared_file_info_v1,
        Shared_file_upload,
        Shared_file_unshared,
        Add_section_option,
        Client_stats_v2,
        Add_plugin_option,
        Client_stats_v3,
        File_info_v2, // 40
        DownloadFiles_v3,
        DownloadedFiles_v1,
        File_info_v3,
        DownloadFiles_v4,
        DownloadedFiles_v2,
        File_downloaded,
        BadPassword,
        Shared_file_info,
        Client_stats,
        File_remove_source, // 50
        CleanTables,
        File_info,
        DownloadFiles,
        DownloadedFiles,
        UploadFiles,
        Pending,
        Search,
        Version // this is new
    };

    //! Opcodes from GUI messages.
    enum ToCore {
        GuiProtocol = 0,
        ConnectMore_query,
        CleanOldServers,
        KillServer,
        ExtendedSearch,
        Password_v1,
        Search_query_v1,
        Download_query_v1,
        Url,
        RemoveServer_query,
        SaveOptions_query, // 10
        RemoveDownload_query,
        ServerUsers_query,
        SaveFile,
        AddClientFriend,
        AddUserFriend,
        RemoveFriend,
        RemoveAllFriends,
        FindFriend,
        ViewUsers,
        ConnectAll, // 20
        ConnectServer,
        DisconnectServer,
        SwitchDownload,
        VerifyAllChunks,
        Query_Format,
        ModifyMp3Tags,
        ForgetSearch, // same as CloseSearch
        SetOption,
        Command,
        Preview, // 30
        ConnectFriend,
        GetServer_users,
        GetClient_files,
        GetFile_locations,
        GetServer_info,
        GetClient_info,
        GetFile_info,
        GetUser_info,
        SendMessage,
        EnableNetwork, // 40
        BrowseUser,
        Search_query,
        MessageToClient,
        GetConnectedServers,
        GetDownloadFiles,
        GetDownloadedFiles,
        GuiExtensions,
        SetRoomState,
        RefreshUploadStats,
        Download_query, // 50
        SetFilePriority,
        Password,
        CloseSearch,
        AddServer_query,
        MessageVersions,
        RenameFile,
        GetUploaders,
        GetPending,
        GetSearches,
        GetSearch, // 60
        ConnectClient,
        DisconnectClient,
        NetworkMessage,
        InterestedInSources,
        GetVersion, // this is new
        ServerRename,
        ServerSetPreferred

    };

    //! Search types.
    enum SearchType {
        LocalSearch = 0,
        RemoteSearch,
        SubscribeSearch
    };

    //! Constructor.
    /*!
     * \param poll If true, the object will enable the poll extension when
     *   connecting to the core. This will cause the core to send you regular
     *   Client_stats messages and very little else, unless you ask for it.
     *   Useful for anything that isn't meant to be a full-fledged GUI.
     * \param parent The parent object.
     */
    DonkeyProtocol(bool poll = false, QObject* parent = 0);
    //! Destructor.
    ~DonkeyProtocol();

    bool poll() const;

    //! Set the authentication credentials.
    /*!
     * Set the username and password the object will send to the core when connecting.
     * You should do this _before_ initiating a connection.
     *
     * \param username The username.
     * \param pwd The password. If an empty string, no password will be sent.
     * \sa setPassword(QString&), username(), password(), connectToCore()
     */
    void setPassword(const QString& username, const QString& pwd);

    //! Set the authentication password.
    /*!
     * Set the password the object will send to the core when connecting.
     * You should do this _before_ initiating a connection.
     *
     * Please note: Using this method sets the username to "admin", which
     * is the default username when connecting to a protocol 14 core
     * (mldonkey 2.03 or higher).
     *
     * \param pwd The password. If an empty string, no password will be sent.
     * \sa setPassword(QString&, QString&), password(), connectToCore()
     */
    void setPassword(const QString& pwd);

    //! Get the authentication password.
    /*!
     * \return The authentication password set with setPassword().
     * \sa setPassword()
     */
    const QString& password();

    //! Get the authentication username.
    /*!
     * \return The username set with setPassword().
     * \sa setPassword(QString&, QString&)
     */
    const QString& username();

    //! Get if the socket is connected
    /*!
     * \return socket is connected (true) or disconnected (false)
     */
    bool isConnected();

    //! Return the current state of the socket
    /*!
     * \return the socket state
     */
    QAbstractSocket::SocketState connectionState() const;

    //! (Re-)Connect to the mlnet-core.
    /*!
     * Initiate a connection to the core.
     */
    bool connectToCore();

    //! Disconnect from the mlnet-core.
    /*!
     * Initiate a disconnection from the core. The signal donkeyDisconnected(int)
     * will be emitted when the disconnect is complete.
     */
    bool disconnectFromCore();

    //! Get the active protocol version.
    /*!
     * When connecting to the core, the DonkeyProtocol object and the core agree
     * on which version of the protocol to use. This will be the highest version
     * of the protocol supported by both parties. This method is used to discover
     * the protocol version currently in use.
     * \return The current protocol version.
     * \sa coreProtocolVersion()
     */
    const int& protocolVersion();

    //! Get the core's protocol version.
    /*!
     * Obtain the protocol version that the connected core requested. This should
     * be the highest version that the core is able to support, and may be different
     * from the protocol version actually being used.
     * \return The core's protocol version.
     * \sa protocolVersion()
     */
    const int& coreProtocolVersion();

    //! Get the currently downloading files.
    /*!
     * \return A dictionary containing FileInfo objects representing the
     *   currently downloading files, indexed on their fileNo() values.
     */
    const FileInfoHash& downloadFiles();

    //! Get the completed files.
    /*!
     * \return A dictionary containing FileInfo objects representing the
     *   completed files, indexed on their fileNo() values.
     */
    const FileInfoHash& downloadedFiles();

    //! Get the connected servers.
    /*!
     * \return A dictionary containing ServerInfo objects representing the
     *   currently connected servers, indexed on their serverNo() values.
     */
    const ServerInfoHash& connectedServers();

    //! Get the network list.
    /*!
     * \return A dictionary containing Network objects representing the
     *   available networks, indexed on their networkNo() values.
     */
    const NetworkHash& availableNetworks();

    //! Get the client list.
    /*!
     * \return A dictionary containing ClientInfo objects representing
     *   the clients, indexed on their clientNo() values.
     */
    const ClientInfoHash& clientList();

    //! Get the shared file list.
    /*!
     * \return A dictionary containing ShareInfo objects representing
     *   the shares, indexed on their shareNo() values.
     */
    const ShareInfoHash& sharedFiles();

    //! Get the list of active searches.
    /*!
     * \return A dictionary containing SearchInfo objects representing
     *   the active searches, indexed on their searchNo() values.
     */
    const SearchInfoHash& activeSearches() const;

    //! Find a downloading file by its file number.
    /*!
     * \param fileno The file number to search for.
     * \return A pointer to the FileInfo object describing the requested
     *   file number, or NULL if there was no such file number.
     * \sa downloadFiles(), findDownloadFileByHash()
     */
    FileInfo* findDownloadFileNo(int fileno);

    //! Find a downloaded file by its file number.
    /*!
     * \param fileno The file number to search for.
     * \return A pointer to the FileInfo object describing the requested
     *   file number, or NULL if there was no such file number.
     * \sa downloadedFiles()
     */
    FileInfo* findDownloadedFileNo(int fileno);

    //! Find a connected server by its server number.
    /*!
     * \param serverno The server number to search for.
     * \return A pointer to the ServerInfo object describing the requested
     *   server, or NULL if there was no such server.
     * \sa connectedServers()
     */
    ServerInfo* findServerNo(int serverno);

    //! Find a network by its network number.
    /*!
     * \param nwno The network number to search for.
     * \return A pointer to the Network object describing the requested
     *   network, or NULL if there was no such network.
     * \sa availableNetworks()
     */
    Network* findNetworkNo(int nwno);
    /**
     * Find a network by its name. By design operates at linear time.
     * \param name The network name reported in Network_info message
     * \return A pointer to the Network object describing the requested
     *   network, or NULL if there was no such network.
     * \sa findNetworkNo
     */
    Network *findNetworkName(const QString &name);

    //! Find a client by its client number.
    /*!
     * \param clno The client number to search for.
     * \return A pointer to the ClientInfo object describing the requested
     *   client, or NULL if there was no such client.
     * \sa clientList()
     */
    ClientInfo* findClientNo(int clno);

    //! Find a shared file by its share number.
    /*!
     * \param shno The share number to search for.
     * \return A pointer to the ShareInfo object describing the requested
     *   share, or NULL if there was no such share.
     * \sa sharedFiles()
     */
    ShareInfo* findShareNo(int shno);

    //! Find a search by its search number.
    /*!
     * \param sno The search number to search for.
     * \return A pointer to the SearchInfo object describing the requested
     *   search, or NULL if there was no such search.
     * \sa activeSearches()
     */
    SearchInfo* findSearchNo(int sno);

    //! Find an unmapped search by its search number.
    SearchInfo* findUnmappedSearchNo(int sno);

    //! Find a client share by its result number.
    /*! This effectively searches the search results that haven't been assigned to a
     *  search. The core reports client shares as search results, but doesn't link
     *  them to any search.
     * \param fileno The result number to search for.
     * \return A pointer to the ResultInfo object describing the requested file,
     *   or NULL if there was no such file.
     * \sa clientFileListing()
     */
    const ResultInfo* findClientFile(int fileno);


    //! Save a file.
    /*!
     * Ask the core to save a file under the given name.
     * \param fileno The file number of the file to save.
     * \param name The name under which the file will be saved.
     */
    void saveFile(int fileno, const QString& name);

    //! Pause or resume a downloading file.
    /*!
     * Ask the core to pause or resume a file.
     * \param fileno The file number to operate on.
     * \param pause True to pause the file, false to resume it.
     */
    void pauseFile(int fileno, bool pause);

    //! Cancel a downloading file.
    /*!
     * Ask the core to abort downloading the given file.
     * \param fileno The file number of the file to cancel.
     */
    void cancelFile(int fileno);

    //! Set the download priority of a file.
    /*!
     * Tell the core to adjust a file's download priority.
     * \param fileno The file number to be adjusted.
     * \param pri The new priority value.
     */
    void setFilePriority(int fileno, int pri);

    //! Retry connecting to a file's sources.
    /*!
     * Ask the core to attempt to reconnect to all known sources
     * for a file.
     * \param fileno The file number to retry.
     */
    void retryFile(int fileno);

    //! Verify a file's chunks.
    /*!
     * Ask the core to verify a file's downloaded chunks.
     * \param fileno The file number to verify.
     */
    void verifyFileChunks(int fileno);

    //! Request the format of a file.
    /*!
     * Ask the core to attempt to determine a file's format.
     * \param fileno The file number to examine.
     */
    void getFileFormat(int fileno);

    //! Preview a file.
    /*!
     * Ask the core to launch the previewer for the given file.
     * \param fileno The file number to preview.
     */
    void previewFile(int fileno);

    //! Submit an URL for downloading.
    /*!
     * Submit an URL to the core for downloading.
     * \param url The URL to download.
     */
    void submitUrl(const QString& url);

    void submitUrl(const QUrl &url);

    //! Send a DonkeyMessage to the core.
    /*!
     * Dispatch a DonkeyMessage to the core. This is a low level
     * method and should be used with caution.
     * \param msg The message to send.
     */
    void sendMessage(const DonkeyMessage& msg);

    //! Ask the core to connect to more servers.
    void connectMoreServers();

    //! Ask the core to remove expired servers.
    void cleanOldServers();

    //! Remove a server from the server list.
    /*! \param serverno The number of the server to remove.
     */
    void removeServer(int serverno);

    //! Blacklist the server.
    /*! \param serverno The number of the server to blacklist.
     */
    void blacklistServer(int serverno);

    //! Ask the core to send updated server info for a given server.
    /*! \param serverno The number of the server to request info for.
     */
    void getServerInfo(int serverno);

    //! Ask the core to send a list of users connected to a server.
    /*! \param serverno The number of the server to request an user list for.
     */
    void getServerUsers(int serverno);

    //! Ask the core to connect to a given server.
    /*! \param serverno The server to connect to.
     */
    void connectServer(int serverno);

    //! Ask the core to disconnect from a given server.
    /*! \param serverno The server to disconnect from.
     */
    void disconnectServer(int serverno);

    //! Add a server to the server list.
    /*! \param network The network this server belongs to.
     *  \param ip The IP address of the server.
     *  \param port The port number of the server.
     */
    void addServer(int network, const QString& ip, int16 port);

    //! Returns the default search name.
    const QString& definedSearch() const;
    //! Returns the defined searches.
    const QMap<QString, SearchQuery*>& definedSearches() const;

    //! Start a search.
    /*! \param searchNum    Each search should have its own identifier.
     *  \param query        The search query to submit.
     *  \param maxHits      Maximum number of results.
     *  \param searchType   The type of search to perform.
     *  \param network      The network to search, or 0 to search all networks.
     */
    void startSearch(int searchNum,
                     SearchQuery* query,
                     int maxHits,
                     SearchType searchType,
                     int network = 0);

    //! Stop a search and discard search results.
    /*! \param searchNum The search ID to stop.
     */
    void stopSearch(int searchNum);

    //! Removes a search from the list of activeSearches.
    /*! \param searchNum The search ID to remove.
     */
    void removeSearch(int searchNum);

    /**
     * Get pending searches.
     */
    void getPending();

    /**
     * Request an update of the list of active searches.
     */
    void getSearches();

    /**
     * Request an update for the given search.
     * @param search The search ID.
     */
    void getSearch(int search);

    void extendedSearch();

    //! Start to download a file.
    /*! \param names  List of filenames
     *  \param num    Download ID
     *  \param force  Force downloading the files
     */
    void startDownload(const QStringList& names, int num, bool force);

    //! Refresh file information.
    /*! Ask the core to send the information for the given file again,
     *  to allow the protocol object to update it in case it has changed.
     *  \param fileno The number of the file to refresh
     */
    void refreshFileInfo(int fileno);

    //! Refresh client information.
    /*! Ask the core to resend the information for the given client,
     *  to allow the protocol object to update it in case it has changed.
     *  \param clientno The number of the client to refresh
     */
    void refreshClientInfo(int clientno);

    //! Fetch informations about the location of a file.
    void getFileLocations(int fileno);

    //! Get the number of servers the core is currently connected to.
    /*! \return The number of connected servers.
     */
    uint connectedServerCount();

    //! Get the number of servers in the core's server list.
    /*! \return The total number of servers in the server list.
     */
    uint totalServerCount();

    //! Get a list of friend clients.
    /*! \return A list of ints corresponding to the client IDs of the current friends.
     */
    const QList<int>& friendList();
    //! Search for a friend.
    /*! \param name The name of the friend to search for.
     */
    void searchForFriend(const QString& name);
    //! Add a client to the friend list.
    /*! \param client The client number to make a friend.
     */
    void addClientFriend(int client);
    //! Remove a friend from the friend list.
    /*! \param client The client number of the friend to remove.
     */
    void removeFriend(int client);
    //! Remove all friends from the friend list.
    void removeAllFriends();
    //! Attempt to connect to a friend.
    /*! \param client The client number of the friend to connect to.
     */
    void connectFriend(int client);

    //! Returns the MLDonkey options.
    /*! \return a QMap<QString,QString> list of key=value pairs
     */
    const QMap<QString, QString>& optionsList();

    //! Get an MLDonkey option.
    /*! Obtain the value of the specified MLDonkey option.
     *  \param option The name of the option
     *  \return The value of the option, or an empty string if it doesn't exist
     */
    QString getOption(const QString& option);

    //! Set an MLDonkey option.
    /*! Tell the core to set the given option to the given value.
     *  \param option The name of the option to set
     *  \param value The value to assign to the named option
     */
    void setOption(const QString& option, const QString& value);

     //! Set and save MLDonkey options.
     /* \param opts key=value option pairs
      */
     void setOptions(const QMap<QString, QString>& opts);

     //! Enable or disable an available network
     /* \param nwno QIntDict<Network> networks-number of the network
      * \param enable Enable or disable the network
      */
     void enableNetwork(int nwno, bool enable);

    /**
     * Return a list of section options.
     * \return A list of section options, defined by DonkeyOption objects
     */
    const QList<DonkeyOption>& sectionOptions();

    /**
     * Return a list of plugin options.
     * \return A list of plugin options, defined by DonkeyOption objects
     */
    const QList<DonkeyOption>& pluginOptions();

    /**
     * Kill the MLDonkey core.
     */
    void killCore();

    /**
     * Send a private message to a client.
     * \param client The numerical ID of the recipient.
     * \param message The message content.
     */
    void sendPrivateMessage(int client, const QString& message);

    /**
     * Returns the RoomInfo-Instance with the id roomno.
     * \param roomno The numerical id of the room. Use findRoomNo() to
     *        get the to this id matching RoomInfo-instance.
     * \return The to the roomno matching RoomInfo-instance or 0 if there
     *         didn't exists such a roomno.
     */
    RoomInfo* findRoomNo(int roomno);

    /**
     * Change the roomstate of a room.
     * \param roomno The numerical id of the room.
     * \param state The state the room should be changed to.
     */
    void setRoomState(int roomno, RoomInfo::RoomState state);

    /**
     * Rename a file.
     * \param fileno The numerical ID of the file to rename.
     * \param name The new name of the file.
     */
    void renameFile(int fileno, const QString& name);

    /**
     * Refresh the uploaders list.
     */
    void updateUploaders();

    /**
     * Ask the core to connect to the given client.
     * @param client The client ID to connect to.
     */
    void connectClient(int client);

    /**
     * Ask the core to disconnect from the given client.
     * @param client The client ID to disconnect from.
     */
    void disconnectClient(int client);

    /**
     * Send a network message.
     * @param network The network ID to send to.
     * @param msg The message to send.
     */
    void sendNetworkMessage(int network, const QString& msg);

    /**
     * Tell the core whether we're interested in learning about file sources.
     * @param interested If false, messages about sources will not be sent by the core.
     */
    void interestedInSources(bool interested);

    /**
     * Request the core's version.
     */
    void requestVersion();

    /**
     * Returns the socket used to connect with the core.
     */
    QAbstractSocket* socket() const;

public slots:

    //! Request a new download file list.
    /*!
     * Ask the core to send an updated download file list.
     * \sa updatedDownloadFiles()
     */
    void updateDownloadFiles();

    //! Request a new downloaded file list.
    /*!
     * Ask the core to send an updated list of completed files.
     * \sa updatedDownloadedFiles()
     */
    void updateDownloadedFiles();

    //! Request a new list of connected servers.
    /*!
     * Ask the core to send an updated list of connected servers.
     * \sa updatedConnectedServers()
     */
    void updateConnectedServers();

    //! Request an updated list of shared files.
    /*!
     * Ask the core to send an updated list of shared files.
     */
    void refreshShared();

    //! Send a console message to the core.
    /*!
     * Dispatch a console message to the core. The result will be sent
     * as a consoleMessage() signal when it arrives if callback == 0.
     * If callback is defined, then the result will be send through
     * callback->callback( result );
     * /param msg consoleMessage()
     * /param callback callback-function. Single object can be used as callback for several
     * messages. DonkeyProtocol takes ownership and will delete callback when it's no longer
     * linked to any message.
     */
    void sendConsoleMessage(const QString& msg, ConsoleCallbackInterface* callback = 0);

    /**
     * Rename a server.
     * @param id The server ID.
     * @param name The server's new name.
     */
    void renameServer(int id, const QString& name);

    /**
     * Change a server's preferred flag.
     * @param id The server ID.
     * @param preferred The server's new preferred flag.
     */
    void setServerPreferred(int id, bool preferred);

protected slots:

    //! Process any messages in the DonkeySocket queue.
    void processMessage();

private slots:
    void socketDisconnected();
    void socketStateChanged(QAbstractSocket::SocketState socketState);
    void socketError(QAbstractSocket::SocketError err);
    void socketProxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);

private:
    void pruneClientRecord(int clientno, FromCore reason);
    void pruneFileRecord(int filenum);
    void flushState();

    DonkeySocket *m_socket;
    QString uname, passwd;
    FileInfoHash download;
    FileInfoHash downloaded;
    QMap<int,bool> upload;
    ServerInfoHash servers;
    NetworkHash networks;
    ClientInfoHash clients;
    ShareInfoHash shares;
    RoomInfoHash rooms;
    SearchInfoHash searches;
    SearchInfoHash unmappedSearches;
    ResultInfoHash unmappedResults;
    QList<DonkeyOption> m_sectionOptions;
    QList<DonkeyOption> m_pluginOptions;
    QMap<QString,QString> options;
    QHash<QString,ConsoleCallbackInterface*> consoleCallbacks;
    QMap<int,int> clientstatsmap;

    enumDisconnectError donkeyError;
    int proto, coreProto;
    uint connectedservers;
    bool wantpoll, downloadstarted;
    QList<int> friends;

    QString defSearch;
    QMap<QString, SearchQuery*> defSearches;

signals:
    //! The state of our used socket to connect with the host changed.
    /*!
     * Emitted whenever the state of the used socket changed.
     * \param state The socketnew state.
     */
    void stateChanged(QAbstractSocket::SocketState state);
    //! A message has been received from the core.
    /*!
     * This is a low level function and should be used with caution.
     * The signal is emitted before the DonkeyProtocol object attempts
     * to parse the message.
     * \param message The message that was received.
     */
    void messageReceived(const DonkeyMessage* message);
    //! Core has sent updated client stats.
    /*!
     * \param ul Bytes uploaded.
     * \param dl Bytes downloaded.
     * \param sh Bytes being shared.
     * \param nsh Number of files being shared.
     * \param tul TCP upload rate.
     * \param tdl TCP download rate.
     * \param uul UDP upload rate.
     * \param udl UDP download rate.
     * \param ndl Number of files being downloaded.
     * \param ncp Number of completed files waiting to be committed.
     * \param nws A map of networks with their number of connected servers as values.
     */
    void clientStats(int64 ul, int64 dl, int64 sh, int nsh, int tul, int tdl, int uul, int udl, int ndl, int ncp, QMap<int,int>* nws);
    //! Updated list of downloading files has been received.
    /*!
     * \sa updateDownloadFiles()
     */
    void updatedDownloadFiles();
    //! Updated list of completed files has been received.
    /*!
     * \sa updateDownloadedFiles()
     */
    void updatedDownloadedFiles();

    //! An updated uploading file has been received.
    /*!
     * \param clientnum The Client-ID.
     */
    void uploadUpdated(int clientnum);
    //! An remove an uploading file has been received.
    /*!
     * \param clientnum The Client-ID.
     */
    void uploadRemoved(int clientnum);

    //! Updated list of connected servers has been received.
    /*!
     * \sa updateConnectedServers()
     */
    void updatedConnectedServers();
    //! A console message has arrived.
    /*!
     * \param msg The text of the received console message.
     * \sa sendConsoleMessage()
     */
    void consoleMessage(const QString& msg);
    //! A file source has been updated.
    /*! \param file The file number.
     *  \param source The source number.
     */
    void fileSourceUpdated(int file, int source);
    //! A file source has been removed.
    /*! \param file The file number.
     *  \param source The source number.
     */
    void fileSourceRemoved(int file, int source);
    //! A file has been updated.
    /*! \param file The file number.
     *  \param reason The Gui-protocol message that lead to the update. */
    void fileUpdated(int file, DonkeyProtocol::FromCore reason);
    //! A new file has been added.
    /*! \param file The file number.
     * \param downloadstarted Newly added downloadfile. */
    void fileAdded(int file, bool downloadstarted);
    //! A file has been removed.
    /*! \param file The file number. */
    void fileRemoved(int file);
    //! A client has updated information (or has just been added).
    /*! \param client The client number. */
    void clientUpdated(int client);
    ///! A client has been removed.
    /*! \param client The client number. */
    void clientRemoved(int client);
    //! A server has updated information.
    /*! \param server The server number. */
    void serverUpdated(int server);
    //! A server has been removed.
    /*! \param server The server number. */
    void serverRemoved(int server);
    //! A shared file has updated information.
    /*! \param share The share number. */
    void shareUpdated(int share);
    //! A shared file has been unshared.
    /*! \param share The share number. */
    void shareRemoved(int share);

    //! Network information has been updated.
    /*!
     * \param network The number of the network that was updated.
     * \sa findNetworkNo()
     */
    void networkUpdated(int network);
    //! A friend has been updated.
    /*! \param client The client number of the updated friend. */
    void friendUpdated(int client);
    //! A friend has been removed.
    /*! \param client The client number of the removed friend. */
    void friendRemoved(int client);
    //! A directory listing has been received from a client.
    /*! This will be followed by a number of searchUpdated() signals keyed to
     *  the result ID in this signal, constituting the contents of the given
     *  directory.
     *  \param client The number of the client involved.
     *  \param directory The name of the directory we are receiving.
     *  \param result The search result ID the contents of this directory will be sent as.
     */
    void clientFileListing(int client, const QString& directory, int result);

    //! A message was received from a client.
    /*! \param client The number of the client who sent the message.
     *  \param message The message that was received.
     */
    void messageFromClient(int client, const QString& message);

    //! A Search-Result arrived.
    /*! \param searchnum The search number.
     *  \param resultinfos The ResultInfo-Objects
     */
    void searchUpdated(int searchnum, QList<ResultInfo*> resultinfos);

    //! The defined searches got updated.
    /*!
     * This signal is emitted if a FromCore::DefineSearches message
     * arrived and the searchfields (like complex, audio, video, etc.)
     * got updated.
     */
    void definedSearchesUpdated();

    //! An unhandled message has been received.
    /*!
     * This signal is emitted when the DonkeyProtocol object receives a
     * message it doesn't know how to handle.
     * \param message The unhandled message.
     */
    void unhandledMessage(const DonkeyMessage* message);

    /**
     * A new section option has been defined by the core.
     *
     * This signal is emitted when the core sends a message defining
     * an option it wants in the regular MLDonkey config dialog.
     * \param option The option that was defined.
     */
    void newSectionOption(const DonkeyOption& option);

    /**
     * A new plugin option has been defined by the core.
     *
     * This signal is emitted when the core sends a message defining
     * an option it wants in the plugin config dialog.
     * \param option The option that was defined.
     */
    void newPluginOption(const DonkeyOption& option);

    /**
     * Options have been updated.
     *
     * This signal is emitted if the core sends a Options_info-message.
     */
    void optionsUpdated();

    /**
     * A room have been updated.
     *
     * This signal is emitted if the core sends a Room_info-message.
     * \param roomno The unique number of the updated room.
     */
    void roomUpdated(int roomno);

    /**
     * A new user entered a room.
     * \param roomno The unique number of the room the user belongs to.
     * \param userno The unique number of the user.
     */
    void roomAddUser(int roomno, int userno);

    /**
     * A user in a room left.
     * \param roomno The unique number of the room the user belongs to.
     * \param userno The unique number of the user.
     */
    void roomRemoveUser(int roomno, int userno);

    /**
     * A general server message arrived.
     * \param roomno The unique number of the room the message is for.
     * \param roommsg The RoomMessage-object holding the message.
     */
    void roomMessage(int roomno, RoomMessage* roommsg);

    /**
     * The requested information about a search has arrived.
     * @param searchid The ID of the search.
     */
    void searchRequest(int searchid);

    /**
     * A core version string has arrived.
     * @param versionString The core's version string.
     */
    void coreVersion(const QString& versionString);

};

#endif
