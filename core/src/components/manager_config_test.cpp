#include <components/manager_config.hpp>

#include <server/handlers/handler_config.hpp>

#include <algorithm>

#include <utest/utest.hpp>

namespace {

constexpr char kConfig[] = R"(
components_manager:
  components:
    api-firebase:
      fcm-send-base-url: $fcm_send_base_url
      fcm-subscribe-base-url: $fcm_subscribe_base_url
    auth-checker-settings: null
    testsuite-support: null
    device-notify-stat: null
    handler-inspect-requests:
      path: /service/inspect-requests
      task_processor: monitor-task-processor
    handler-log-level:
      path: /service/log-level/*
      method: GET,PUT
      task_processor: monitor-task-processor
    handler-implicit-http-options:
      as_fallback: implicit-http-options
      task_processor: main-task-processor
      auth_checkers:
        type: tvm2
    handler-ping:
      path: /ping
      url_trailing_slash: strict-match
      task_processor: main-task-processor
    handler-send:
      path: /v1/send
      task_processor: main-task-processor
    handler-server-monitor:
      path: /
      task_processor: monitor-task-processor
    handler-subscribe:
      path: /v1/subscribe
      task_processor: main-task-processor
    handler-unsubscribe:
      path: /v1/unsubscribe
      task_processor: main-task-processor
    http-client: null
    http-server-settings: null
    logging:
      fs-task-processor-name: fs-task-processor
      loggers:
        access:
          file_path: /var/log/yandex/taxi-device-notify/access.log
          overflow_behavior: discard
          pattern: '[%Y-%m-%d %H:%M:%S.%f %z] %v'
        access-tskv:
          file_path: /var/log/yandex/taxi-device-notify/access_tskv.log
          overflow_behavior: discard
          pattern: "tskv\ttskv_format=taxi_device_notify\ttimestamp=%Y-%m-%dT%H:%M:%S\t\
            timezone=%z%v"
        default:
          file_path: /var/log/yandex/taxi-device-notify/server.log
          level: $logger_level
          level#fallback: info
          overflow_behavior: discard
    manager-controller: null
    postgresql-devicenotify:
      blocking_task_processor: pg-task-processor
      dbalias: devicenotify
    secdist:
      config: /etc/yandex/taxi-secdist/taxi.json
    server:
      listener:
        connection:
          in_buffer_size: 32768
          requests_queue_size_threshold: 100
        port: $server_port
        port#fallback: 1180
        task_processor: main-task-processor
      listener-monitor:
        connection:
          in_buffer_size: 32768
          requests_queue_size_threshold: 100
        port: $monitor_server_port
        port#fallback: 1188
        task_processor: monitor-task-processor
      logger_access: ''
      logger_access_tskv: ''
    statistics-storage: null
    taxi-config:
      bootstrap-path: /etc/yandex/taxi/device-notify/taxi_config_bootstrap.json
      fs-cache-path: /var/cache/yandex/taxi-device-notify/config_cache.json
      fs-task-processor-name: fs-task-processor
    taxi-config-client-updater:
      config-settings: false
      config-url: $config_server_url
      fallback-path: /etc/yandex/taxi/device-notify/taxi_config_fallback.json
      full-update-interval: 1m
      http-retries: 5
      http-timeout: 1000ms
      load-enabled: true
      load-only-my-values: true
      store-enabled: true
      update-interval: 5s
    tests-control:
      enabled: false
      path: /tests/control
      task_processor: main-task-processor
    tracer:
      tracer: native
    worker-cleanup-inactive-users:
      cleanup-inactive-users-period: $cleanup_inactive_users_period
      task_processor: bg-task-processor
    worker-fallback-queue:
      cleanup-period: $fallback_cleanup_period
      message-delay: $fallback_message_delay
      read-period: $fallback_read_period
      task_processor: bg-task-processor
    worker-fallback-subscription-queue:
      retry-delay: $fallback_subscription_retry_delay
      task-period: $fallback_subscription_period
      task_processor: bg-task-processor
  coro_pool:
    initial_size: $coro_pool_initial_size
    initial_size#fallback: 5000
    max_size: $coro_pool_max_size
    max_size#fallback: 50000
  default_task_processor: main-task-processor
  event_thread_pool:
    threads: $event_threads
    threads#fallback: 2
  task_processors:
    bg-task-processor:
      thread_name: bg-worker
      worker_threads: $bg_worker_threads
      worker_threads#fallback: 2
    fs-task-processor:
      thread_name: fs-worker
      worker_threads: $fs_worker_threads
    main-task-processor:
      thread_name: main-worker
      worker_threads: $main_worker_threads
    monitor-task-processor:
      thread_name: monitor-worker
      worker_threads: $monitor_worker_threads
    pg-task-processor:
      thread_name: pg-worker
      worker_threads: $pg_worker_threads
      worker_threads#fallback: 2
)";

constexpr char kVariables[] = R"(
bg_worker_threads: 4
cleanup_inactive_users_period: 1s
config_server_url: localhost:9999/configs-service
#coro_pool_initial_size: 25000
coro_pool_max_size: 10000
event_threads: 3
fallback_cleanup_period: 1s
fallback_message_delay: 1m
fallback_read_period: 1s
fallback_subscription_period: 1s
fallback_subscription_retry_delay: 1m
fcm_send_base_url: http://localhost:9999
fcm_subscribe_base_url: http://localhost:9999
fs_worker_threads: 4
logger_level: info
main_worker_threads: 16
monitor_worker_threads: 4
pg_worker_threads: 4
redis_threads: 8
)";

}  // namespace

TEST(ManagerConfig, Basic) {
  auto vmap = std::make_shared<yaml_config::VariableMap>(
      formats::yaml::FromString(kVariables));

  components::ManagerConfig mc = components::ManagerConfig::ParseFromYaml(
      formats::yaml::FromString(kConfig), "components_manager", vmap);

  EXPECT_EQ(mc.default_task_processor, "main-task-processor");
  EXPECT_EQ(mc.coro_pool.max_size, 10000) << "config vars do not work";
  EXPECT_EQ(mc.coro_pool.initial_size, 5000) << "#fallback does not work";
  EXPECT_EQ(mc.task_processors.size(), 5);

  ASSERT_EQ(mc.components.size(), 27);
  EXPECT_EQ(mc.components.front().Name(), "api-firebase");
  EXPECT_EQ(mc.components.back().Name(), "worker-fallback-subscription-queue");
}

TEST(ManagerConfig, HandlerConfig) {
  auto vmap = std::make_shared<yaml_config::VariableMap>(
      formats::yaml::FromString(kVariables));

  components::ManagerConfig mc = components::ManagerConfig::ParseFromYaml(
      formats::yaml::FromString(kConfig), "components_manager", vmap);

  const auto it =
      std::find_if(mc.components.cbegin(), mc.components.cend(),
                   [](const auto& v) { return v.Name() == "tests-control"; });
  ASSERT_NE(it, mc.components.cend()) << "failed to find 'tests-control'";

  EXPECT_EQ(it->FullPath(), "components_manager.components.tests-control");

  server::handlers::HandlerConfig conf =
      server::handlers::HandlerConfig::ParseFromYaml(it->Yaml(), it->FullPath(),
                                                     it->ConfigVarsPtr());

  EXPECT_EQ(std::get<std::string>(conf.path), "/tests/control");
  EXPECT_EQ(conf.task_processor, "main-task-processor");
}
