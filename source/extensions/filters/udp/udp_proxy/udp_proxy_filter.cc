#include "extensions/filters/udp/udp_proxy/udp_proxy_filter.h"

#include "envoy/network/listener.h"

#include "common/network/utility.h"

namespace Envoy {
namespace Extensions {
namespace UdpFilters {
namespace UdpProxy {

void UdpProxyFilter::onData(Network::UdpRecvData& data) {
  // fixfix peer and local address?
  const auto active_session_it = sessions_.find(*data.peer_address_);
  ActiveSession* active_session;
  if (active_session_it == sessions_.end()) {
    // fixfix keep track of cluster.
    Upstream::ThreadLocalCluster* cluster = config_->getCluster();
    if (cluster == nullptr) {
      ASSERT(false); // fixfix
    }

    // fixfix pass context
    Upstream::HostConstSharedPtr host = cluster->loadBalancer().chooseHost(nullptr);
    if (host == nullptr) {
      ASSERT(false); // fixfix
    }

    // fixfix deal with host going away or failing health checks.
    auto new_session = std::make_unique<ActiveSession>(*this, std::move(data.peer_address_), host);
    active_session = new_session.get();
    sessions_.emplace(std::move(new_session));
  } else {
    active_session = active_session_it->get();
  }

  active_session->write(*data.buffer_, *data.local_address_);
}

UdpProxyFilter::ActiveSession::ActiveSession(
    UdpProxyFilter& parent, Network::Address::InstanceConstSharedPtr&& source_address,
    const Upstream::HostConstSharedPtr& host)
    : parent_(parent), source_address_(std::move(source_address)), host_(host),
      idle_timer_(parent.read_callbacks_->udpListener().dispatcher().createTimer(
          [this] { onIdleTimer(); })),
      io_handle_(host->address()->socket(Network::Address::SocketType::Datagram)),
      socket_event_(parent.read_callbacks_->udpListener().dispatcher().createFileEvent(
          io_handle_->fd(), [this](uint32_t) { onReadReady(); }, Event::FileTriggerType::Edge,
          Event::FileReadyType::Read)) {
  // fixfix start idle timer.
}

void UdpProxyFilter::ActiveSession::onIdleTimer() { parent_.sessions_.erase(*source_address_); }

void UdpProxyFilter::ActiveSession::onReadReady() {
  // fixfix idle timer.
  ASSERT(false);
}

void UdpProxyFilter::ActiveSession::write(const Buffer::Instance& buffer,
                                          const Network::Address::Instance& local_address) {
  // fixfix idle timer.
  Buffer::RawSlice slice;
  const uint32_t num_slices = buffer.getRawSlices(&slice, 1);
  ASSERT(num_slices == 1);
  Api::IoCallUint64Result rc = Network::Utility::writeToSocket(
      *io_handle_, &slice, 1, local_address.ip(), *host_->address());
  ASSERT(rc.ok()); // fixfix
}

} // namespace UdpProxy
} // namespace UdpFilters
} // namespace Extensions
} // namespace Envoy
