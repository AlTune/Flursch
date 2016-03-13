#define dprintf(M, ...) printf("[DEBUG] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)

static struct libusb_device_handle *device;

int RecoveryMode;

int kDFUMode;

char* serial_str;

char* nonce_str;

char* imei_str;

char* srnm_str;

/* MobileDevice API */
extern int AMDeviceConnect (void *device);
extern int AMDeviceIsPaired (void *device);
extern int AMDeviceValidatePairing (void *device);
extern int AMDeviceStartSession (void *device);
extern int AMDeviceEnterRecovery (void *device);
extern int AMDeviceNotificationSubscribe(void *, int , int , int, void **);
struct AMDeviceNotificationCallbackInformation {
    void 		*deviceHandle;
    uint32_t	msgType;
} ;

void enterrecovery(struct AMDeviceNotificationCallbackInformation *CallbackInfo);

int enterdfu();

int send_buffer(unsigned char* data, int len, int mode);

int send_file(const char* filename, int mode);

int send_cmd(char* command);

int limera1n();

int steaks4uce();

int connect2(int pid, int attempts);

void libflursch_exit();

void libflursch_init();

void get_serial();

void get_nonce();

int get_cpid();

int get_cprv();

int get_cpfm();

int get_scep();

int get_bdid();

int get_ibfl();

unsigned long long get_ecid();

float get_srtg();

void get_srnm();

void get_imei();

int file_exists(const char* fileName);

int inject();