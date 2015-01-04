/*
 * Copyright (C) Tildeslash Ltd. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 *
 * You must obey the GNU Affero General Public License in all respects
 * for all of the code used other than OpenSSL.  
 */


/**
 * Implementation of the Network Statistics for Linux.
 *
 * @author http://www.tildeslash.com/
 * @see http://www.mmonit.com/
 * @file
 */


static boolean_t _update(T L, const char *interface) {
        char buf[STRLEN];
        char path[PATH_MAX];
        /*
         * Get interface operation state (Optional: may not be present on older kernels).
         * $ cat /sys/class/net/eth0/operstate
         * up
         */
        snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", interface);
        FILE *f = fopen(path, "r");
        if (f) {
                if (fscanf(f, "%256s\n", buf) == 1) {
                        L->state = Str_isEqual(buf, "down") ? 0LL : 1LL;
                }
                fclose(f);
        }
        /*
         * Get interface speed (Optional: may not be present on older kernels).
         * $ cat /sys/class/net/eth0/speed
         * 1000
         */
        snprintf(path, sizeof(path), "/sys/class/net/%s/speed", interface);
        f = fopen(path, "r");
        if (f) {
                int speed;
                if (fscanf(f, "%d\n", &speed) == 1) {
                        L->speed = speed * 1000000; // mbps -> bps
                }
                fclose(f);
        }
        /*
         * Get interface full/half duplex status (Optional: may not be present on older kernels).
         * $ cat /sys/class/net/eth0/duplex
         * full
         */
        snprintf(path, sizeof(path), "/sys/class/net/%s/duplex", interface);
        f = fopen(path, "r");
        if (f) {
                if (fscanf(f, "%256s\n", buf) == 1) {
                        L->duplex = Str_isEqual(buf, "full") ? 1LL : 0LL;
                }
                fclose(f);
        }
        /*
         * $ cat /proc/net/dev
         * Inter-|   Receive                                                |  Transmit
         *  face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
         *   eth0: 1841444   11557    0    0    0     0          0         0  1335636    7725    0    0    0     0       0          0
         *     lo:   28760     200    0    0    0     0          0         0    28760     200    0    0    0     0       0          0
         */
        f = fopen("/proc/net/dev", "r");
        if (f) {
                long long ibytes, ipackets, ierrors, obytes, opackets, oerrors;
                while (fgets(buf, sizeof(buf), f) != NULL) {
                        char iface[STRLEN];
                        if (sscanf(buf, "%256[^:]: %lld %lld %lld %*s %*s %*s %*s %*s %lld %lld %lld %*s %*s %*s %*s %*s", iface, &ibytes, &ipackets, &ierrors, &obytes, &opackets, &oerrors) == 7 && Str_isEqual(Str_trim(iface), interface)) {
                                L->timestamp.last = L->timestamp.now;
                                L->timestamp.now = Time_milli();
                                L->ipackets.last = L->ipackets.now;
                                L->ipackets.now = ipackets;
                                L->ibytes.last = L->ibytes.now;
                                L->ibytes.now = ibytes;
                                L->ierrors.last = L->ierrors.now;
                                L->ierrors.now = ierrors;
                                L->opackets.last = L->opackets.now;
                                L->opackets.now = opackets;
                                L->obytes.last = L->obytes.now;
                                L->obytes.now = obytes;
                                L->oerrors.last = L->oerrors.now;
                                L->oerrors.now = oerrors;
                                fclose(f);
                                return true;
                        }
                }
                fclose(f);
        } else {
                THROW(AssertException, "Cannot read /proc/net/dev -- %s", System_getError(errno));
        }
        return false;
}

