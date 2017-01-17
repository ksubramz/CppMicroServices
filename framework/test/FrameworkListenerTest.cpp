/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "TestingConfig.h"
#include "TestingMacros.h"
#include "TestUtils.h"

#include <thread>
#include <vector>

using namespace cppmicroservices;

namespace {

class TestFrameworkListener
{
public:
  TestFrameworkListener() : _events() {}
  virtual ~TestFrameworkListener() {};

  std::size_t events_received() const { return _events.size(); }

  bool CheckEvents(const std::vector<FrameworkEvent>& events)
  {
    bool listenState = true; // assume success
    if (events.size() != _events.size())
    {
      listenState = false;
      US_TEST_OUTPUT( << "*** Framework event mismatch ***\n expected "
                        << events.size() << " event(s)\n found "
                        << _events.size() << " event(s).");

      const std::size_t max = events.size() > _events.size() ? events.size() : _events.size();
      for (std::size_t i = 0; i < max; ++i)
      {
        const FrameworkEvent& pE = i < events.size() ? events[i] : FrameworkEvent();
        const FrameworkEvent& pR = i < _events.size() ? _events[i] : FrameworkEvent();
        US_TEST_OUTPUT( << " - " << pE << " - " << pR);
      }
    }
    else
    {
      for (std::size_t i = 0; i < events.size(); ++i)
      {
        const FrameworkEvent& pE = events[i];
        const FrameworkEvent& pR = _events[i];
        if (pE.GetType() != pR.GetType() || pE.GetBundle() != pR.GetBundle())
        {
          listenState = false;
          US_TEST_OUTPUT( << "*** Wrong framework event ***\n found " << pR << "\n expected " << pE);
        }
      }
    }

    _events.clear();
    return listenState;
  }

  void frameworkEvent(const FrameworkEvent& evt)
  {
    _events.push_back(evt);
    std::cout << evt << std::endl;
  }

  void throwOnFrameworkEvent(const FrameworkEvent&)
  {
    throw std::runtime_error("whoopsie!");
  }

private:
  std::vector<FrameworkEvent> _events;
};

void testStartStopFrameworkEvents()
{
  auto f = FrameworkFactory().NewFramework();

  TestFrameworkListener l;
  f.Init();
  f.GetBundleContext().AddFrameworkListener(&l, &TestFrameworkListener::frameworkEvent);
  f.Start();
  f.Stop();

  std::vector<FrameworkEvent> events;
  events.push_back(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_STARTED, f, "Framework Started"));
  US_TEST_CONDITION_REQUIRED(l.CheckEvents(events), "Test for the correct number and order of Framework start/stop events.");
}

void testAddRemoveFrameworkListener()
{
  auto f = FrameworkFactory().NewFramework();
  f.Init();
  BundleContext fCtx{ f.GetBundleContext() };

  // Test that the lambda is removed correctly if the lambda is referenced by a variable
  int count{ 0 };
  auto listener = [&count](const FrameworkEvent&) { ++count; };
  fCtx.AddFrameworkListener(listener);
  fCtx.RemoveFrameworkListener(listener);

  // test listener removal...
  TestFrameworkListener l;
  fCtx.AddFrameworkListener(&l, &TestFrameworkListener::frameworkEvent);
  fCtx.RemoveFrameworkListener(&l, &TestFrameworkListener::frameworkEvent);

  f.Start();   // generate framework event
  US_TEST_CONDITION(l.CheckEvents(std::vector<FrameworkEvent>()), "Test listener removal");
  US_TEST_CONDITION(count == 0, "Test listener removal");
  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());

  count = 0;
  f.Init();
  fCtx = f.GetBundleContext();
  auto fl = [&count](const FrameworkEvent& ) { ++count; };
  fCtx.AddFrameworkListener(fl);

  f.Start();
  US_TEST_CONDITION(count == 1, "Test listener addition");

  fCtx.RemoveFrameworkListener(fl);
  // note: The Framework STARTED event is only sent once. Stop and Start the framework to generate another one.
  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());

  f.Init();
  fCtx = f.GetBundleContext();
  fCtx.AddFrameworkListener(&l, &TestFrameworkListener::frameworkEvent);
  f.Start();
  US_TEST_CONDITION(l.CheckEvents(std::vector<FrameworkEvent>{FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_STARTED, f, "Framework Started")}), "Test listener addition");
  US_TEST_CONDITION(count == 1, "Test listener was successfully removed");
  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());

  // @fixme issue #95 ... can't add more than one lambda defined listener
  // uncomment this block once issue #95 is fixed.
  //int count1(0);
  //int count2(0);
  //auto listener_callback_counter1 = [&count1](const FrameworkEvent&) { ++count1; std::cout << "listener_callback_counter1: call count " << count1 << std::endl; };
  //auto listener_callback_counter2 = [&count2](const FrameworkEvent&) { ++count2; std::cout << "listener_callback_counter2: call count " << count2 << std::endl; };
  //auto listener_callback_throw = [](const FrameworkEvent&) { throw std::runtime_error("boo"); };
  //
  //f.Init();
  //fCtx = f.GetBundleContext();
  //fCtx.AddFrameworkListener(listener_callback_counter1);
  //fCtx.AddFrameworkListener(listener_callback_counter2);
  //fCtx.AddFrameworkListener(listener_callback_throw);

  //f.Start();    // generate framework event (started)
  //US_TEST_CONDITION(count1 == 1, "Test that multiple framework listeners were called");
  //US_TEST_CONDITION(count2 == 1, "Test that multiple framework listeners were called");

  //fCtx.RemoveFrameworkListener(listener_callback_counter1);
  //fCtx.RemoveFrameworkListener(listener_callback_counter2);
  //fCtx.RemoveFrameworkListener(listener_callback_throw);

  //f.Start();    // generate framework event (started)
  //US_TEST_CONDITION(count1 == 1, "Test that multiple framework listeners were NOT called after removal");
  //US_TEST_CONDITION(count2 == 1, "Test that multiple framework listeners were NOT called after removal");
  // end @fixme issue #95
}

void callback_function_1(const FrameworkEvent&)
{
  std::cout << "From free function callback_function_1" << std::endl;
}

void callback_function_2(const FrameworkEvent&)
{
  std::cout << "From free function callback_function_2" << std::endl;
}

void callback_function_3(int val, const FrameworkEvent&)
{
  std::cout << "From free function callback_function_3 with val " << val << std::endl;
}

class CallbackFunctor
{
public:
  void operator()(const FrameworkEvent&)
  {
    std::cout << "From function object of type CallbackFunctor " << std::endl;
  }
};

class Listener
{
public:
  void memfn1(const FrameworkEvent&)
  {
    std::cout << "From member function Listener::memfn1" << std::endl;
  }

  void memfn2(const FrameworkEvent&)
  {
    std::cout << "From member function Listener::memfn2" << std::endl;
  }
};

void testMultipleListeners()
{
  auto lambda1 = [](const FrameworkEvent&) { std::cout << "From lambda1" << std::endl; };
  auto lambda2 = [](const FrameworkEvent&) { std::cout << "From lambda2" << std::endl; };
  CallbackFunctor cb;
  Listener l1;
  Listener l2;

  auto f = FrameworkFactory().NewFramework();

  // 1. Add all listeners
  f.Init();
  BundleContext fCtx{ f.GetBundleContext() };
  fCtx.AddFrameworkListener(callback_function_1);
  fCtx.AddFrameworkListener(&callback_function_2);
  fCtx.AddFrameworkListener(&l1, &Listener::memfn1);
  fCtx.AddFrameworkListener(&l2, &Listener::memfn2);
  fCtx.AddFrameworkListener(cb);
  fCtx.AddFrameworkListener(lambda1);
  fCtx.AddFrameworkListener(lambda2);
  fCtx.AddFrameworkListener(CallbackFunctor());
  fCtx.AddFrameworkListener(std::bind(callback_function_3, 42, std::placeholders::_1));
  f.Start();    // generate framework event (started)
  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
  std::cout << "-- End of testing addition of multiple listeners" << std::endl << std::endl;

  // 2. Add all listeners and try removing listeners using their name
  f.Init();
  fCtx = f.GetBundleContext();
  // Add listeners of each variety
  // Listeners with distinct addresses
  fCtx.AddFrameworkListener(callback_function_1);
  fCtx.AddFrameworkListener(&callback_function_2);
  fCtx.AddFrameworkListener(&l1, &Listener::memfn1);
  fCtx.AddFrameworkListener(&l2, &Listener::memfn2);
  fCtx.AddFrameworkListener(cb);
  // Listeners of the other variety
  fCtx.AddFrameworkListener(lambda1);
  fCtx.AddFrameworkListener(lambda2);
  fCtx.AddFrameworkListener(CallbackFunctor());
  auto bind1 = std::bind(callback_function_3, 42, std::placeholders::_1);
  fCtx.AddFrameworkListener(bind1);

  // Remove listeners with distinct addresses using the way how
  // they were added. They return true if they are successful.
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(callback_function_1), "Removing free function 1");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(&callback_function_2), "Removing free function 2");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(&l1, &Listener::memfn1), "Removing member function of l1");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(&l2, &Listener::memfn2), "Removing member function of l2");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(cb), "Removing functor cb");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(bind1), "Removing bind object bind1");
  // Remove listeners using the name of lambdas. They fail and it's indicated by returning false
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(lambda1) == false, "Removing lambda1 fails and returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(lambda2) == false, "Removing lambda2 fails and returns false");
  // Remove the distinct address listeners again. These should be no-op and the operation returns false
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(callback_function_1) == false,
                    "Removing free function 1 again fails and returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(&callback_function_2) == false,
                    "Removing free function 2 again fails and returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(&l1, &Listener::memfn1) == false,
                    "Removing member function of l1 again fails and returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(&l2, &Listener::memfn2) == false,
                    "Removing member function of l2 again fails and returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(cb) == false,
                    "Removing functor cb again fails and returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(bind1) == false,
                    "Removing bind object bind1 again fails and returns false");
  // This should trigger only the 3 non-distinct addresses listeners i.e. the 2 lambda functions, the
  // rvalue functor object
  f.Start();    // generate framework event (started)
  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
  std::cout << "-- End of testing removing listeners using the name of the callable" << std::endl << std::endl;

  // 3. Add all listeners and remove them using tokens
  f.Init();
  fCtx = f.GetBundleContext();
  auto token1 = fCtx.AddFrameworkListener(callback_function_1);
  auto token2 = fCtx.AddFrameworkListener(&callback_function_2);
  auto token3 = fCtx.AddFrameworkListener(&l1, &Listener::memfn1);
  auto token4 = fCtx.AddFrameworkListener(&l2, &Listener::memfn2);
  auto token5 = fCtx.AddFrameworkListener(cb);
  auto token6 = fCtx.AddFrameworkListener(lambda1);
  auto token7 = fCtx.AddFrameworkListener(lambda2);
  auto token8 = fCtx.AddFrameworkListener(CallbackFunctor());
  auto token9 = fCtx.AddFrameworkListener(bind1);
  // Remove all added listeners using tokens. These should all return true because of successful removal.
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token1), "Removing listener associated with token1");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token2), "Removing listener associated with token2");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token3), "Removing listener associated with token3");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token4), "Removing listener associated with token4");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token5), "Removing listener associated with token5");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token6), "Removing listener associated with token6");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token7), "Removing listener associated with token7");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token8), "Removing listener associated with token8");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token9), "Removing listener associated with token9");
  // Remove all added listeners again using token. These should all return false because the listeners
  // associated with these tokens have already been removed.
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token1) == false,
                    "Removing listener associated with token1 again returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token2) == false,
                    "Removing listener associated with token2 again returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token3) == false,
                    "Removing listener associated with token3 again returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token4) == false,
                    "Removing listener associated with token4 again returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token5) == false,
                    "Removing listener associated with token5 again returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token6) == false,
                    "Removing listener associated with token6 again returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token7) == false,
                    "Removing listener associated with token7 again returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token8) == false,
                    "Removing listener associated with token8 again returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token9) == false,
                    "Removing listener associated with token9 again returns false");
  // This should result in no output because all the listeners were successfully removed
  f.Start();    // generate framework event (started)
  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
  std::cout << "-- End of testing addition and removing listeners using tokens" << std::endl << std::endl;

  // 4. Add and remove multiple non-static member function listeners
  f.Init();
  fCtx = f.GetBundleContext();
  fCtx.AddFrameworkListener(&l1, &Listener::memfn1);
  fCtx.AddFrameworkListener(&l1, &Listener::memfn2);
  fCtx.AddFrameworkListener(&l2, &Listener::memfn1);
  fCtx.AddFrameworkListener(&l2, &Listener::memfn2);
  // Removing these listeners by name fails (and returns false)
  // because removing more than one member function listener from the same object is ambiguous
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(&l1, &Listener::memfn1) == false,
                    "Removing member function fails and returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(&l1, &Listener::memfn2) == false,
                    "Removing member function fails and returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(&l2, &Listener::memfn1) == false,
                    "Removing member function fails and returns false");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(&l2, &Listener::memfn2) == false,
                    "Removing member function fails and returns false");
  // This should result in all 4 member functions getting triggered.
  f.Start();    // generate framework event (started)
  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
  std::cout << "-- End of testing removing multiple member function listeners using the name" << std::endl << std::endl;

  // 5. Add and remove multiple non-static member function listeners, but this time using tokens
  f.Init();
  fCtx = f.GetBundleContext();
  token1 = fCtx.AddFrameworkListener(&l1, &Listener::memfn1);
  token2 = fCtx.AddFrameworkListener(&l1, &Listener::memfn2);
  token3 = fCtx.AddFrameworkListener(&l2, &Listener::memfn1);
  token4 = fCtx.AddFrameworkListener(&l2, &Listener::memfn2);
  // Remove these listeners using the tokens
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token1), "Removing member function associated with token1");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token2), "Removing member function associated with token2");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token3), "Removing member function associated with token3");
  US_TEST_CONDITION(fCtx.RemoveFrameworkListener(token4), "Removing member function associated with token4");
  // This should result in no output because there are all the registered listeners were
  // successfully removed using tokens
  f.Start();    // generate framework event (started)
  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
  std::cout << "-- End of testing removing multiple member function listeners using tokens" << std::endl << std::endl;

  // 6. Add same listeners multiple times for listeners with distinct addresses
  f.Init();
  fCtx = f.GetBundleContext();
  token1 = fCtx.AddFrameworkListener(callback_function_1);
  token2 = fCtx.AddFrameworkListener(callback_function_1);
  token3 = fCtx.AddFrameworkListener(&callback_function_2);
  token4 = fCtx.AddFrameworkListener(&callback_function_2);
  token5 = fCtx.AddFrameworkListener(cb);
  token6 = fCtx.AddFrameworkListener(cb);
  token7 = fCtx.AddFrameworkListener(bind1);
  token8 = fCtx.AddFrameworkListener(bind1);
  // Test if adding listener with distinct address again returns the same token object
  US_TEST_CONDITION(token1 == token2, "Adding distinct address listener again should return the same token");
  US_TEST_CONDITION(token3 == token4, "Adding distinct address listener again should return the same token");
  US_TEST_CONDITION(token5 == token6, "Adding distinct address listener again should return the same token");
  US_TEST_CONDITION(token7 == token8, "Adding distinct address listener again should return the same token");
  // This results in addition of only one listener of each variety because listeners with
  // distinct addresses aren't added again if they already exist. There's a total of 4 listeners
  f.Start();    // generate framework event (started)
  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
  std::cout << "-- End of testing adding listeners with distinct addresses multiple times" << std::endl << std::endl;

  // 7. Add same listeners multiple times for other type of listeners
  f.Init();
  fCtx = f.GetBundleContext();
  auto lambda3 = [](const FrameworkEvent&) { std::cout << "From lambda3" << std::endl; };
  token1 = fCtx.AddFrameworkListener(lambda3);
  token2 = fCtx.AddFrameworkListener(lambda3);
  token3 = fCtx.AddFrameworkListener(&l1, &Listener::memfn1);
  token4 = fCtx.AddFrameworkListener(&l1, &Listener::memfn1);
  token5 = fCtx.AddFrameworkListener(&l1, &Listener::memfn2);
  token6 = fCtx.AddFrameworkListener(&l1, &Listener::memfn2);
  token7 = fCtx.AddFrameworkListener(&l2, &Listener::memfn1);
  token8 = fCtx.AddFrameworkListener(&l2, &Listener::memfn1);
  token9 = fCtx.AddFrameworkListener(&l2, &Listener::memfn2);
  auto token10 = fCtx.AddFrameworkListener(&l2, &Listener::memfn2);
  // Test if adding listener again returns a different token
  US_TEST_CONDITION(token1 != token2, "Adding this type of listener again should return different token");
  US_TEST_CONDITION(token3 != token4, "Adding this type of listener again should return different token");
  US_TEST_CONDITION(token5 != token6, "Adding this type of listener again should return different token");
  US_TEST_CONDITION(token7 != token8, "Adding this type of listener again should return different token");
  US_TEST_CONDITION(token9 != token10, "Adding this type of listener again should return different token");
  // This results in 10 listeners getting called
  f.Start();    // generate framework event (started)
  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
  std::cout << "-- End of testing adding listeners multiple times" << std::endl << std::endl;
}

void testFrameworkListenersAfterFrameworkStop()
{
  auto f = FrameworkFactory().NewFramework();
  f.Init();
  // OSGi section 10.2.2.13 (Framework::stop API):
  //    4. Event handling is disabled.
  //    6. All resources held by this Framework are released.
  // The assumption is that framework listeners are one such resource described in step #6.
  int events(0);
  auto listener = [&events](const FrameworkEvent& evt) { ++events; std::cout << evt << std::endl; };
  f.GetBundleContext().AddFrameworkListener(listener);
  f.Start();    // generate framework event (started)
  f.Stop();     // resources (such as framework listeners) are released
  // due to the asynchronous nature of Stop(), we must wait for the stop to complete
  // before starting the framework again. If this doesn't happen, the start may send
  // a framework event before the listener is disabled and cleaned up.
  f.WaitForStop(std::chrono::milliseconds::zero());
  f.Start();    // generate framework event (started) with no listener to see it
  US_TEST_CONDITION(events == 1 , "Test that listeners were released on Framework Stop");
}

void testFrameworkListenerThrowingInvariant()
{
  /*
    The Framework must publish a FrameworkEvent.ERROR if a callback to an event listener generates an exception - except
    when the callback happens while delivering a FrameworkEvent.ERROR (to prevent an infinite loop).

    Tests:
    1. Given a bundle listener which throws -> verfiy a FrameworkEvent ERROR is received with the correct event info.
    2. Given a service listener which throws -> verfiy a FrameworkEvent ERROR is received with the correct event info.
    3. Given a framework listener which throws -> No FrameworkEvent is received, instead an internal log message is sent.
  */
  std::stringstream logstream;
  std::ostream sink(logstream.rdbuf());
  // Use a redirected logger to verify that the framework listener logged an
  // error message when it encountered a FrameworkEvent::ERROR coming from
  // a framework listener.
  auto f = FrameworkFactory().NewFramework(std::map<std::string, cppmicroservices::Any>{ { Constants::FRAMEWORK_LOG, true } }, &sink);
  f.Init();

  bool fwk_error_received(false);
  std::string exception_string("bad callback");
  auto listener = [&fwk_error_received, &exception_string](const FrameworkEvent& evt)
    {
      try
      {
        if(evt.GetThrowable()) std::rethrow_exception(evt.GetThrowable());
      }
      catch (const std::exception& e)
      {
        if (FrameworkEvent::Type::FRAMEWORK_ERROR == evt.GetType() &&
            e.what() == exception_string &&
            std::string(typeid(e).name()) == typeid(std::runtime_error).name())
        {
          fwk_error_received = true;
        }
      }
    };

  f.GetBundleContext().AddFrameworkListener(listener);
  // @todo A STARTING BundleEvent should be sent before the Framework runs its Activator (in Start()). Apache Felix does it this way.
  f.Start();

  // Test #1 - test bundle event listener
  auto bl = [](const BundleEvent&) { throw std::runtime_error("bad callback"); };
  f.GetBundleContext().AddBundleListener(bl);
  auto bundleA2 = testing::InstallLib(f.GetBundleContext(), "TestBundleA2");    // generate a bundle event for shared libs
#ifndef US_BUILD_SHARED_LIBS
  US_TEST_CONDITION(bundleA2, "TestBundleA2 bundle not found");
  bundleA2.Start(); // since bundles are auto-installed, start the bundle to generate a bundle event
#endif
  US_TEST_CONDITION(fwk_error_received, "Test that a Framework ERROR event was received from a throwing bundle listener");
  f.GetBundleContext().RemoveBundleListener(bl);

  // Test #2 - test service event listener
  fwk_error_received = false;
  exception_string = "you sunk my battleship";
  auto sl = [](const ServiceEvent&) { throw std::runtime_error("you sunk my battleship");  };
  f.GetBundleContext().AddServiceListener(sl);
  auto bundleA = testing::InstallLib(f.GetBundleContext(), "TestBundleA");
  bundleA.Start();  // generate a service event
  US_TEST_CONDITION(fwk_error_received, "Test that a Framework ERROR event was received from a throwing service listener");
  f.GetBundleContext().RemoveServiceListener(sl);

  // note: The Framework STARTED event is only sent once. Stop and Start the framework to generate another one.
  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());

  // Test #3 - test framework event listener
  f.Init();
  fwk_error_received = false;
  exception_string = "whoopsie!";
  TestFrameworkListener l;
  f.GetBundleContext().RemoveFrameworkListener(listener);   // remove listener until issue #95 is fixed.
  f.GetBundleContext().AddFrameworkListener(&l, &TestFrameworkListener::throwOnFrameworkEvent);
  // This will cause a deadlock if this test fails.
  f.Start();    // generates a framework event
  US_TEST_CONDITION(false == fwk_error_received, "Test that a Framework ERROR event was NOT received from a throwing framework listener");
  US_TEST_CONDITION(std::string::npos != logstream.str().find("A Framework Listener threw an exception:"), "Test for internal log message from Framework event handler");

}

#ifdef US_ENABLE_THREADING_SUPPORT
void testDeadLock()
{
  // test for deadlocks during Framework API re-entry from a Framework Listener callback
  auto f = FrameworkFactory().NewFramework();
  f.Start();

  auto listener = [&f](const FrameworkEvent& evt)
  {
    if (FrameworkEvent::Type::FRAMEWORK_ERROR == evt.GetType())
    {
        // generate a framework event on another thread,
        // which will cause a deadlock if any mutexes are locked.
        // Doing this on the same thread would produce
        // undefined behavior (typically a deadlock or an exception)
        std::thread t([&f]() {try { f.Start(); } catch (...) {} });
        t.join();
    }
  };

  f.GetBundleContext().AddBundleListener([](const BundleEvent&) { throw std::runtime_error("bad bundle"); });
  f.GetBundleContext().AddFrameworkListener(listener);
  auto bundleA = testing::InstallLib(f.GetBundleContext(), "TestBundleA"); // trigger the bundle listener to be called

  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
}
#endif

} // end anonymous namespace

int FrameworkListenerTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("FrameworkListenerTest");

  auto lambda1 = [](const FrameworkEvent&) {};
  auto lambda2 = [](const FrameworkEvent&) {};
  US_TEST_CONDITION((typeid(lambda1) != typeid(lambda2)), "Test lambda type info (in)equality");
  US_TEST_CONDITION(
      std::function<void(const FrameworkEvent&)>(lambda1).target<void(const FrameworkEvent&)>() == std::function<void(const FrameworkEvent&)>(lambda2).target<void(const FrameworkEvent&)>(),
      "Test std::function target equality"
  );

  /*
    @note Framework events will be sent like service events; synchronously.
          Framework events SHOULD be delivered asynchronously (see OSGi spec), however that's not yet supported.
    @todo test asynchronous event delivery once its supported.
  */

  //testStartStopFrameworkEvents();
  //testAddRemoveFrameworkListener();
  //testFrameworkListenersAfterFrameworkStop();
  //testFrameworkListenerThrowingInvariant();
#ifdef US_ENABLE_THREADING_SUPPORT
  //testDeadLock();
#endif

  testMultipleListeners();

  US_TEST_END()
}
