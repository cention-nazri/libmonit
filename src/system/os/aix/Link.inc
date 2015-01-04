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
 * Implementation of the Network Statistics for AIX.
 *
 * @author http://www.tildeslash.com/
 * @see http://www.mmonit.com/
 * @file
 */


static boolean_t _update(T L, const char *interface) {
        perfstat_id_t id;
        perfstat_netinterface_t buf;
        snprintf(id.name, sizeof(id.name), interface);
        if (perfstat_netinterface(&id, &buf, sizeof(buf), 1) != 1)
                THROW(AssertException, "Cannot get perfstat data for %s -- %s", interface, System_getError(errno));
        L->ipackets.last = L->ipackets.now;
        L->ibytes.last = L->ibytes.now;
        L->ierrors.last = L->ierrors.now;
        L->opackets.last = L->opackets.now;
        L->obytes.last = L->obytes.now;
        L->oerrors.last = L->oerrors.now;
        L->speed = buf.bitrate;
        L->ipackets.now = buf.ipackets;
        L->ibytes.now = buf.ibytes;
        L->ierrors.now = buf.ierrors;
        L->opackets.now = buf.opackets;
        L->obytes.now = buf.obytes;
        L->oerrors.now = buf.oerrors;
        L->timestamp.last = L->timestamp.now;
        L->timestamp.now = Time_milli();
        //FIXME: L->state and L->duplex are not implemented
        return true;
}

